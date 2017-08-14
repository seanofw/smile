
sub LoadTemplate {
	my $templateFileName = $_[0];

	open (my $file, '<:encoding(UTF-8)', $templateFileName)
		or die "Could not open file '$templateFileName' for reading";

	@lines = ();
	while (my $line = <$file>) {
		chomp $line;
		push @lines, $line;
	}
	
	close $file;
	
	return @lines;
}

sub ApplyTemplate {
	my $template = $_[0];
	my %substitutions = %{$_[1]};
	
	foreach $templateVariableName (keys %substitutions) {
		$replacement = $substitutions{$templateVariableName};
		$needle = quotemeta "%$templateVariableName%";
		$template =~ s/$needle/$replacement/g;
	}
	
	return $template;
}

sub OutputTemplate {
	my @template = @{$_[0]};
	my %substitutions = %{$_[1]};
	my $outputFileName = $_[2];
	
	print "$outputFileName\n";
	open (my $file, '>encoding(UTF-8)', $outputFileName)
		or die "Could not open file '$outputFileName' for writing";
	
	print $file "// ===================================================\n";
	print $file "//   WARNING: THIS IS A GENERATED FILE. DO NOT EDIT!\n";
	print $file "// ===================================================\n\n";
	
	foreach $line (@template) {
		foreach $templateVariableName (keys %substitutions) {
			$replacement = $substitutions{$templateVariableName};
			$needle = quotemeta "%$templateVariableName%";
			$line =~ s/$needle/$replacement/g;
		}
		print $file "$line\n";
	}

	close $file;
}

#--------------------------------------------------------------------------------------------

@extTemplate = LoadTemplate("eval_fn_ext.template");
@userTemplate = LoadTemplate("eval_fn_user.template");

#--------------------------------------------------------------------------------------------

# This is the prototype for eall of the fast-args functions.
$fastUserFunctionTemplate = <<EOI;
void %name%(SmileFunction self, Int argc, Int extra)
{
	UserFunctionInfo userFunctionInfo;
	Closure childClosure;

	if (argc != %numArgs%) {
		Smile_ThrowException(Smile_KnownSymbols.native_method_error, String_Format(_argCountErrorMessage,
			SMILE_VCALL1(self, toString, (SmileUnboxedData){ 0 }), %numArgs%, argc));
	}

	/* Create a new child closure for this function. */
	userFunctionInfo = self->u.u.userFunctionInfo;
	childClosure = Closure_CreateLocal(&userFunctionInfo->closureInfo, self->u.u.declaringClosure,
		_closure, _segment, _byteCode - _segment->byteCodes);

	/* Copy the arguments. */
%copyArgs%
	Closure_PopCount(_closure, extra + %numArgs%);

	/* We're now in the child, so set up the globals for running inside it. */
	_closure = childClosure;
	_segment = userFunctionInfo->byteCodeSegment;
	_compiledTables = _segment->compiledTables;
	_byteCode = &_segment->byteCodes[0];
}

EOI

$fastUserFunctionTemplate =~ s/\n/\r\n/g;

# Construct the NoArgs version of the prototype.
%noArgs = ("name" => "SmileUserFunction_NoArgs_Call", "numArgs" => "0", "copyArgs" => "");
$fastUserFunctions = ApplyTemplate($fastUserFunctionTemplate, \%noArgs);

# Construct the code for 1-8 args in fast mode.
for ($numArgs = 1; $numArgs <= 8; $numArgs++) {
	$copyArgs = "";
	for ($i = 0; $i < $numArgs; $i++) {
		$copyArgs .= "\tchildClosure->variables[$i] = _closure->stackTop[" . (-($numArgs - $i)) . "];\r\n";
	}
	%args = ("name" => "SmileUserFunction_Fast" . $numArgs . "_Call", "numArgs" => "$numArgs", "copyArgs" => $copyArgs);

	$fastUserFunctions .= ApplyTemplate($fastUserFunctionTemplate, \%args);
}

# Create the generated C code.
%templateArgs = ("FAST_USER_FUNCTIONS" => $fastUserFunctions);
OutputTemplate(\@userTemplate, \%templateArgs, "eval_fn_user.generated.c");

#--------------------------------------------------------------------------------------------

%templateArgs = (
	"INVOKE_DECL" => "SmileArg *argv = _closure->stackTop - argc;",

	"INVOKE_TYPE_DECL" => "SmileArg *argv = _closure->stackTop - argc;",

	"INVOKE_SM_DECL" => "SmileArg *argv = _closure->stackTop - argc;\r\n"
		. "\tSmileArg stateMachineResult;",

	"INVOKE_SM_TYPE_DECL" => "SmileArg *argv = _closure->stackTop - argc;\r\n"
		. "\tSmileArg stateMachineResult;",

	"DO_MIN_CHECK" => "if (argc < self->u.externalFunctionInfo.minArgs)\r\n"
		. "\t\tThrowMinCheckError(self, argc);\r\n",

	"DO_MAX_CHECK" => "if (argc > self->u.externalFunctionInfo.maxArgs)\r\n"
		. "\t\tThrowMaxCheckError(self, argc);\r\n",

	"DO_EXACT_CHECK" => "if (argc != self->u.externalFunctionInfo.minArgs)\r\n"
		. "\t\tThrowExactCheckError(self, argc);\r\n",

	"DO_TYPE_CHECK" => "PerformTypeChecks(self, argc, argv, self->u.externalFunctionInfo.numArgsToTypeCheck, self->u.externalFunctionInfo.argTypeChecks);\r\n",

	"INVOKE" => "_closure->stackTop = argv - extra;\r\n"
		. "\t*_closure->stackTop++ = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);",

	"INVOKE_STATE_MACHINE" => "_closure->stackTop = argv - extra;\r\n"
		. "\tstateMachineResult = self->u.externalFunctionInfo.externalFunction(argc, argv, self->u.externalFunctionInfo.param);\r\n"
		. "\tif (stateMachineResult.obj != NULL) {\r\n"
		. "\t\t/* Didn't start the state machine, and instead produced a result directly. */\r\n"
		. "\t\t*_closure->stackTop++ = stateMachineResult;\t\n"
		. "\t}"
);

OutputTemplate(\@extTemplate, \%templateArgs, "eval_fn_ext.generated.c");
