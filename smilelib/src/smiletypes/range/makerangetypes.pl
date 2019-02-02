
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

@rangeTemplate = LoadTemplate("smilerange.template");
@rangeBaseTemplate = LoadTemplate("smilerange_base.template");

#--------------------------------------------------------------------------------------------

$intToStringBase10 = <<EOI;
		((obj->end >= obj->start && obj->stepping != +1
			|| obj->end < obj->start && obj->stepping != -1)
			? String_Format("%S..%S step %S",
				String_CreateFromInteger(obj->start, 10, False),
				String_CreateFromInteger(obj->end, 10, False),
				String_CreateFromInteger(obj->stepping, 10, False))
			: String_Format("%S..%S",
				String_CreateFromInteger(obj->start, 10, False),
				String_CreateFromInteger(obj->end, 10, False)))
EOI

$intToString = <<EOI;
		((obj->end >= obj->start && obj->stepping != +1
			|| obj->end < obj->start && obj->stepping != -1)
			? String_Format("%S..%S step %S",
				String_CreateFromInteger(obj->start, (Int)numericBase, False),
				String_CreateFromInteger(obj->end, (Int)numericBase, False),
				String_CreateFromInteger(obj->stepping, (Int)numericBase, False))
			: String_Format("%S..%S",
				String_CreateFromInteger(obj->start, (Int)numericBase, False),
				String_CreateFromInteger(obj->end, (Int)numericBase, False)))
EOI

%int64Substitutions = (
	"type" => "integer64",
	"Type" => "Integer64",
	"TYPE" => "INTEGER64",
	"RawType" => "Int64",
	"URawType" => "UInt64",
	"unboxed" => "unboxed.i64",
	"TypeName" => "An Integer64",
	"ShortType" => "Int64",

	"DeltaType" => "Integer64",
	"DELTATYPE" => "INTEGER64",
	"RawDeltaType" => "Int64",
	"unboxeddelta" => "unboxed.i64",
	"ShortDeltaType" => "Int64",

	"include" => "<smile/smiletypes/numeric/smileinteger64.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "1",
	"Modulus" => "a % b",

	"ToInt32" => "(Int32)(obj->end - obj->start)",
	"ToReal64" => "Real64_FromInt64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"ToStringBase10" => $intToStringBase10,
	"ToString" => $intToString,

	"HashAlgorithm" => <<EOI,
		UInt64 start = (UInt64)range->start;
		UInt64 end = (UInt64)range->end;
		UInt64 stepping = (UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (start >> 32)) ^ (UInt32)(end ^ (end >> 32)) ^ (UInt32)(stepping ^ (stepping >> 32)));
EOI
);

OutputTemplate(\@rangeTemplate, \%int64Substitutions, "smileinteger64range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%int64Substitutions, "smileinteger64range_base.generated.c");

#--------------------------------------------------------------------------------------------

%int32Substitutions = (
	"type" => "integer32",
	"Type" => "Integer32",
	"TYPE" => "INTEGER32",
	"RawType" => "Int32",
	"URawType" => "UInt32",
	"unboxed" => "unboxed.i32",
	"TypeName" => "An Integer32",
	"ShortType" => "Int32",

	"DeltaType" => "Integer32",
	"RawDeltaType" => "Int32",
	"DELTATYPE" => "INTEGER32",
	"unboxeddelta" => "unboxed.i32",
	"ShortDeltaType" => "Int32",

	"include" => "<smile/smiletypes/numeric/smileinteger32.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "1",
	"Modulus" => "a % b",

	"ToInt32" => "(obj->end - obj->start)",
	"ToReal64" => "Real64_FromInt64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"ToStringBase10" => $intToStringBase10,
	"ToString" => $intToString,

	"HashAlgorithm" => <<EOI,
		UInt32 start = (UInt32)range->start;
		UInt32 end = (UInt32)range->end;
		UInt32 stepping = (UInt32)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ end ^ stepping));
EOI
);

OutputTemplate(\@rangeTemplate, \%int32Substitutions, "smileinteger32range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%int32Substitutions, "smileinteger32range_base.generated.c");

#--------------------------------------------------------------------------------------------

%int16Substitutions = (
	"type" => "integer16",
	"Type" => "Integer16",
	"TYPE" => "INTEGER16",
	"RawType" => "Int16",
	"URawType" => "UInt16",
	"unboxed" => "unboxed.i16",
	"TypeName" => "An Integer16",
	"ShortType" => "Int16",

	"DeltaType" => "Integer16",
	"RawDeltaType" => "Int16",
	"DELTATYPE" => "INTEGER16",
	"unboxeddelta" => "unboxed.i16",
	"ShortDeltaType" => "Int16",

	"include" => "<smile/smiletypes/numeric/smileinteger16.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "1",
	"Modulus" => "a % b",

	"ToInt32" => "(obj->end - obj->start)",
	"ToReal64" => "Real64_FromInt64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"ToStringBase10" => $intToStringBase10,
	"ToString" => $intToString,

	"HashAlgorithm" => <<EOI,
		UInt16 start = (UInt16)range->start;
		UInt16 end = (UInt16)range->end;
		UInt16 stepping = (UInt16)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16)));
EOI
);

OutputTemplate(\@rangeTemplate, \%int16Substitutions, "smileinteger16range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%int16Substitutions, "smileinteger16range_base.generated.c");

#--------------------------------------------------------------------------------------------

%byteSubstitutions = (
	"type" => "byte",
	"Type" => "Byte",
	"TYPE" => "BYTE",
	"RawType" => "Byte",
	"URawType" => "Byte",
	"unboxed" => "unboxed.i8",
	"TypeName" => "A Byte",
	"ShortType" => "Byte",

	"DeltaType" => "Byte",
	"RawDeltaType" => "SByte",
	"DELTATYPE" => "BYTE",
	"unboxeddelta" => "unboxed.i8",
	"ShortDeltaType" => "Byte",

	"include" => "<smile/smiletypes/numeric/smilebyte.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "1",
	"Modulus" => "a % b",

	"ToInt32" => "((Int32)obj->end - (Int32)obj->start)",
	"ToReal64" => "Real64_FromInt64((Int32)obj->end - (Int32)obj->start)",
	"ToFloat64" => "(Float64)((Int32)obj->end - (Int32)obj->start)",
	"ToStringBase10" => $intToStringBase10,
	"ToString" => $intToString,

	"HashAlgorithm" => <<EOI,
		Byte start = range->start;
		Byte end = range->end;
		Byte stepping = range->stepping;
		result = Smile_ApplyHashOracle((UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16)));
EOI
);

OutputTemplate(\@rangeTemplate, \%byteSubstitutions, "smilebyterange.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%byteSubstitutions, "smilebyterange_base.generated.c");

#--------------------------------------------------------------------------------------------

$charToString = <<EOI;
	((obj->end >= obj->start && obj->stepping != +1
		|| obj->end < obj->start && obj->stepping != -1)
		? String_Format("'%S'..'%S' step %ld",
			String_AddCSlashes(String_CreateRepeat(obj->start, 1)),
			String_AddCSlashes(String_CreateRepeat(obj->end, 1)),
			obj->stepping)
		: String_Format("'%S'..'%S'",
			String_AddCSlashes(String_CreateRepeat(obj->start, 1)),
			String_AddCSlashes(String_CreateRepeat(obj->end, 1))))
EOI

%charSubstitutions = (
	"type" => "char",
	"Type" => "Char",
	"TYPE" => "CHAR",
	"RawType" => "Byte",
	"URawType" => "Byte",
	"unboxed" => "unboxed.ch",
	"TypeName" => "A Char",
	"ShortType" => "Char",

	"DeltaType" => "Integer64",
	"RawDeltaType" => "Int64",
	"DELTATYPE" => "INTEGER64",
	"unboxeddelta" => "unboxed.i64",
	"ShortDeltaType" => "Int64",

	"include" => "<smile/smiletypes/text/smilechar.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "0",
	"Modulus" => "a % b",

	"ToInt32" => "((Int32)obj->end - (Int32)obj->start)",
	"ToReal64" => "Real64_FromInt64((Int32)obj->end - (Int32)obj->start)",
	"ToFloat64" => "(Float64)((Int32)obj->end - (Int32)obj->start)",
	"ToStringBase10" => $charToString,
	"ToString" => $charToString,

	"HashAlgorithm" => <<EOI,
		Byte start = range->start;
		Byte end = range->end;
		UInt32 stepping = (UInt32)(UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16)));
EOI
);

OutputTemplate(\@rangeTemplate, \%charSubstitutions, "smilecharrange.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%charSubstitutions, "smilecharrange_base.generated.c");

#--------------------------------------------------------------------------------------------

$uniToString = <<EOI;
	((obj->end >= obj->start && obj->stepping != +1
		|| obj->end < obj->start && obj->stepping != -1)
		? String_Format("'\\\\u%X'..'\\\\u%X' step %ld", obj->start, obj->end, obj->stepping)
		: String_Format("'\\\\u%X'..'\\\\u%X'", obj->start, obj->end))
EOI

%uniSubstitutions = (
	"type" => "uni",
	"Type" => "Uni",
	"TYPE" => "UNI",
	"RawType" => "UInt32",
	"URawType" => "UInt32",
	"unboxed" => "unboxed.uni",
	"TypeName" => "A Uni",
	"ShortType" => "Uni",

	"DeltaType" => "Integer64",
	"RawDeltaType" => "Int64",
	"DELTATYPE" => "INTEGER64",
	"unboxeddelta" => "unboxed.i64",
	"ShortDeltaType" => "Int64",

	"include" => "<smile/smiletypes/text/smileuni.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "0",
	"Modulus" => "a % b",

	"ToInt32" => "((Int32)obj->end - (Int32)obj->start)",
	"ToReal64" => "Real64_FromInt64((Int32)obj->end - (Int32)obj->start)",
	"ToFloat64" => "(Float64)((Int32)obj->end - (Int32)obj->start)",
	"ToStringBase10" => $uniToString,
	"ToString" => $uniToString,

	"HashAlgorithm" => <<EOI,
		UInt32 start = range->start;
		UInt32 end = range->end;
		UInt32 stepping = (UInt32)(UInt64)range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (end << 8) ^ (stepping << 16)));
EOI
);

OutputTemplate(\@rangeTemplate, \%uniSubstitutions, "smileunirange.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%uniSubstitutions, "smileunirange_base.generated.c");

#--------------------------------------------------------------------------------------------

$floatToString = <<EOI;
((obj->end >= obj->start && obj->stepping != +1
	|| obj->end < obj->start && obj->stepping != -1)
	? String_Format("(%S)..(%S) step %S",
		Float64_ToStringEx(obj->start, 0, 0, False),
		Float64_ToStringEx(obj->end, 0, 0, False),
		Float64_ToStringEx(obj->stepping, 0, 0, False))
	: String_Format("%S..%S",
		Float64_ToStringEx(obj->start, 0, 0, False),
		Float64_ToStringEx(obj->end, 0, 0, False)))
EOI

%float32Substitutions = (
	"type" => "float32",
	"Type" => "Float32",
	"TYPE" => "FLOAT32",
	"RawType" => "Float32",
	"URawType" => "Float32",
	"unboxed" => "unboxed.f32",
	"TypeName" => "A Float32",
	"ShortType" => "Float32",

	"DeltaType" => "Float32",
	"RawDeltaType" => "Float32",
	"DELTATYPE" => "FLOAT32",
	"unboxeddelta" => "unboxed.f32",
	"ShortDeltaType" => "Float32",

	"include" => "<smile/smiletypes/numeric/smilefloat32.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "0",
	"Modulus" => "fmodf(a, b)",

	"ToInt32" => "(Int32)(obj->end - obj->start)",
	"ToReal64" => "Real64_FromFloat32(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"ToStringBase10" => $floatToString,
	"ToString" => $floatToString,

	"HashAlgorithm" => <<EOI,
		UInt32 start = *(UInt32 *)&range->start;
		UInt32 end = *(UInt32 *)&range->end;
		UInt32 stepping = *(UInt32 *)&range->stepping;
		result = Smile_ApplyHashOracle(start ^ end ^ stepping);
EOI
);

OutputTemplate(\@rangeTemplate, \%float32Substitutions, "smilefloat32range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%float32Substitutions, "smilefloat32range_base.generated.c");

#--------------------------------------------------------------------------------------------

%float64Substitutions = (
	"type" => "float64",
	"Type" => "Float64",
	"TYPE" => "FLOAT64",
	"RawType" => "Float64",
	"URawType" => "Float64",
	"unboxed" => "unboxed.f64",
	"TypeName" => "A Float64",
	"ShortType" => "Float64",

	"DeltaType" => "Float64",
	"RawDeltaType" => "Float64",
	"DELTATYPE" => "FLOAT64",
	"unboxeddelta" => "unboxed.f64",
	"ShortDeltaType" => "Float64",

	"include" => "<smile/smiletypes/numeric/smilefloat64.h>",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"OtherBases" => "0",
	"Modulus" => "fmod(a, b)",

	"ToInt32" => "(Int32)(obj->end - obj->start)",
	"ToReal64" => "Real64_FromFloat64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"ToStringBase10" => $floatToString,
	"ToString" => $floatToString,

	"HashAlgorithm" => <<EOI,
		UInt64 start = *(UInt64 *)&range->start;
		UInt64 end = *(UInt64 *)&range->end;
		UInt64 stepping = *(UInt64 *)&range->stepping;
		result = Smile_ApplyHashOracle((UInt32)(start ^ (start >> 32)) ^ (UInt32)(end ^ (end >> 32)) ^ (UInt32)(stepping ^ (stepping >> 32)));
EOI
);

OutputTemplate(\@rangeTemplate, \%float64Substitutions, "smilefloat64range.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%float64Substitutions, "smilefloat64range_base.generated.c");

#--------------------------------------------------------------------------------------------
