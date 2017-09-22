
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

@realTemplate = LoadTemplate("smilereal.template");
@realBaseTemplate = LoadTemplate("smilereal_base.template");

#--------------------------------------------------------------------------------------------

%real64Substitutions = (
	"type" => "real64",
	"Type" => "Real64",
	"TYPE" => "REAL64",
	"RawType" => "Real64",
	"unboxed" => "r64",
	"numbits" => "64",
	"TypeName" => "A Real64",
	"ToBool" => "!Real64_IsZero(unboxedData.r64)",
	"ToBoolArg" => "!Real64_IsZero(argv[0].unboxed.r64)",
	"ToInt" => "SmileUnboxedInteger64_From(Real64_ToInt64(argv[0].unboxed.r64))",
	"ToStringBase10" => "Real64_ToStringEx(unboxedData.r64, 0, 0, False)",
	"ToStringArg" => "Real64_ToStringEx(argv[0].unboxed.r64, 0, 0, False)",
	"HashAlgorithm" => "(UInt32)(*(UInt64 *)&obj->value ^ (*(UInt64 *)&obj->value >> 32))"
);

OutputTemplate(\@realTemplate, \%real64Substitutions, "smilereal64.generated.c");
OutputTemplate(\@realBaseTemplate, \%real64Substitutions, "smilereal64_base.generated.c");
