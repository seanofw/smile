
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

@rangeTemplate = LoadTemplate("smilerealrange.template");
@rangeBaseTemplate = LoadTemplate("smilerealrange_base.template");

#--------------------------------------------------------------------------------------------

%real32Substitutions = (
	"type" => "real32",
	"Type" => "Real32",
	"TYPE" => "REAL32",
	"RawType" => "Real32",
	"unboxed" => "unboxed.r32",
	"TypeName" => "A Real32",

	"include" => "<smile/smiletypes/numeric/smilereal32.h>",
	"length" => "Real32_Gt(self->end, self->start) ? Real32_Add(Real32_Sub(self->end, self->start), Real32_One) : Real32_Add(Real32_Sub(self->start, self->end), Real32_One)",

	"ToInt32" => "Real32_ToInt32(Real32_Sub(self->end, self->start))",
	"ToReal64" => "Real32_Sub(obj->end, obj->start)",
	"ToFloat64" => "Real32_ToFloat64(Real32_Sub(self->end, self->start))",
	"ToString" => <<EOI,
((Real32_Ge(obj->end, obj->start) && Real32_Ne(obj->stepping, Real32_One)
	|| Real32_Lt(obj->end, obj->start) && Real32_Ne(obj->stepping, Real32_One))
	? String_Format("(%S)..(%S) step %S",
		Real32_ToStringEx(obj->start, 0, 0, False),
		Real32_ToStringEx(obj->end, 0, 0, False),
		Real32_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("(%S)..(%S)",
		Real32_ToStringEx(obj->start, 0, 0, False),
		Real32_ToStringEx(obj->end, 0, 0, False)))
EOI
	"HashAlgorithm" => <<EOI,
		UInt32 start = *(UInt32 *)&range->start;
		UInt32 end = *(UInt32 *)&range->end;
		UInt32 stepping = *(UInt32 *)&range->stepping;
		result = Smile_ApplyHashOracle(start ^ end ^ stepping);
EOI
);

OutputTemplate(\@rangeTemplate, \%real32Substitutions, "smilereal32range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%real32Substitutions, "smilereal32range_base.generated.c");

#--------------------------------------------------------------------------------------------

%real64Substitutions = (
	"type" => "real64",
	"Type" => "Real64",
	"TYPE" => "REAL64",
	"RawType" => "Real64",
	"unboxed" => "unboxed.r64",
	"TypeName" => "A Real64",

	"include" => "<smile/smiletypes/numeric/smilereal64.h>",
	"length" => "Real64_Gt(self->end, self->start) ? Real64_Add(Real64_Sub(self->end, self->start), Real64_One) : Real64_Add(Real64_Sub(self->start, self->end), Real64_One)",

	"ToInt32" => "Real64_ToInt64(Real64_Sub(self->end, self->start))",
	"ToReal64" => "Real64_Sub(obj->end, obj->start)",
	"ToFloat64" => "Real64_ToFloat64(Real64_Sub(self->end, self->start))",
	"ToString" => <<EOI,
((Real64_Ge(obj->end, obj->start) && Real64_Ne(obj->stepping, Real64_One)
	|| Real64_Lt(obj->end, obj->start) && Real64_Ne(obj->stepping, Real64_One))
	? String_Format("(%S)..(%S) step %S",
		Real64_ToStringEx(obj->start, 0, 0, False),
		Real64_ToStringEx(obj->end, 0, 0, False),
		Real64_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("(%S)..(%S)",
		Real64_ToStringEx(obj->start, 0, 0, False),
		Real64_ToStringEx(obj->end, 0, 0, False)))
EOI
	"HashAlgorithm" => <<EOI,
		UInt64 start = *(UInt64 *)&range->start;
		UInt64 end = *(UInt64 *)&range->end;
		UInt64 stepping = *(UInt64 *)&range->stepping;
		UInt64 hash = start ^ end ^ stepping;
		result = Smile_ApplyHashOracle((Int32)(hash ^ (hash >> 32)));
EOI
);

OutputTemplate(\@rangeTemplate, \%real64Substitutions, "smilereal64range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%real64Substitutions, "smilereal64range_base.generated.c");
