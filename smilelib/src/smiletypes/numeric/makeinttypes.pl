#!/usr/bin/perl

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

@integerTemplate = LoadTemplate("smileinteger.template");
@integerBaseTemplate = LoadTemplate("smileinteger_base.template");

#--------------------------------------------------------------------------------------------

%int64Substitutions = (
	"type" => "integer64",
	"Type" => "Integer64",
	"TYPE" => "INTEGER64",
	"RawType" => "Int64",
	"URawType" => "UInt64",
	"unboxed" => "i64",
	"numbits" => "64",
	"TypeName" => "An Integer64",
	"ToInt" => "argv[0]",
	"ToInt64" => "argv[0]",
	"ToInt32" => "SmileUnboxedInteger32_From((Int32)argv[0].unboxed.i64)",
	"ToInt16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i64)",
	"ToByte" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i64)",
	"SignExtend64" => "argv[0]",
	"SignExtend32" => "SmileUnboxedInteger32_From((Int32)argv[0].unboxed.i64)",
	"SignExtend16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i64)",
	"SignExtend8" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i64)",
	"ZeroExtend64" => "argv[0]",
	"ZeroExtend32" => "SmileUnboxedInteger32_From((Int32)argv[0].unboxed.i64)",
	"ZeroExtend16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i64)",
	"ZeroExtend8" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i64)",
	"ToStringBase10" => "String_CreateFromInteger(unboxedData.i64, 10, False)",
	"HashAlgorithm" => "SMILE_APPLY_HASH_ORACLE((UInt64)obj->value ^ ((UInt64)obj->value >> 32))",
	"Sign" => "value == 0 ? SmileUnboxedInteger64_From(0) : value > 0 ? SmileUnboxedInteger64_From(1) : SmileUnboxedInteger64_From(-1)",
	"ToChar" => "SmileUnboxedChar_From((Byte)argv[0].unboxed.i64)",
	"ToUni" => "SmileUnboxedUni_FromSafeInt64(argv[0].unboxed.i64)"
);

OutputTemplate(\@integerTemplate, \%int64Substitutions, "smileinteger64.generated.c");
OutputTemplate(\@integerBaseTemplate, \%int64Substitutions, "smileinteger64_base.generated.c");

#--------------------------------------------------------------------------------------------

%int32Substitutions = (
	"type" => "integer32",
	"Type" => "Integer32",
	"TYPE" => "INTEGER32",
	"RawType" => "Int32",
	"URawType" => "UInt32",
	"unboxed" => "i32",
	"numbits" => "32",
	"TypeName" => "An Integer32",
	"ToInt" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i32)",
	"ToInt64" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i32)",
	"ToInt32" => "argv[0]",
	"ToInt16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i32)",
	"ToByte" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i32)",
	"SignExtend64" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i32)",
	"SignExtend32" => "argv[0]",
	"SignExtend16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i32)",
	"SignExtend8" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i32)",
	"ZeroExtend64" => "SmileUnboxedInteger64_From((Int64)(UInt64)(UInt32)argv[0].unboxed.i32)",
	"ZeroExtend32" => "argv[0]",
	"ZeroExtend16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i32)",
	"ZeroExtend8" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i32)",
	"ToStringBase10" => "String_CreateFromInteger(unboxedData.i32, 10, False)",
	"HashAlgorithm" => "SMILE_APPLY_HASH_ORACLE((UInt32)obj->value)",
	"Sign" => "value == 0 ? SmileUnboxedInteger32_From(0) : value > 0 ? SmileUnboxedInteger32_From(1) : SmileUnboxedInteger32_From(-1)",
	"ToChar" => "SmileUnboxedChar_From((Byte)argv[0].unboxed.i32)",
	"ToUni" => "SmileUnboxedUni_FromSafeInt32(argv[0].unboxed.i32)"
);

OutputTemplate(\@integerTemplate, \%int32Substitutions, "smileinteger32.generated.c");
OutputTemplate(\@integerBaseTemplate, \%int32Substitutions, "smileinteger32_base.generated.c");

#--------------------------------------------------------------------------------------------

%int16Substitutions = (
	"type" => "integer16",
	"Type" => "Integer16",
	"TYPE" => "INTEGER16",
	"RawType" => "Int16",
	"URawType" => "UInt16",
	"unboxed" => "i16",
	"numbits" => "16",
	"TypeName" => "An Integer16",
	"ToInt" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i16)",
	"ToInt64" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i16)",
	"ToInt32" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i16)",
	"ToInt16" => "argv[0]",
	"ToByte" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i16)",
	"SignExtend64" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i16)",
	"SignExtend32" => "SmileUnboxedInteger32_From((Int32)argv[0].unboxed.i16)",
	"SignExtend16" => "argv[0]",
	"SignExtend8" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i16)",
	"ZeroExtend64" => "SmileUnboxedInteger64_From((Int64)(UInt64)(UInt32)argv[0].unboxed.i16)",
	"ZeroExtend32" => "SmileUnboxedInteger32_From((Int64)(UInt64)(UInt32)argv[0].unboxed.i16)",
	"ZeroExtend16" => "argv[0]",
	"ZeroExtend8" => "SmileUnboxedByte_From((Byte)argv[0].unboxed.i16)",
	"ToStringBase10" => "String_CreateFromInteger(unboxedData.i16, 10, False)",
	"HashAlgorithm" => "SMILE_APPLY_HASH_ORACLE((UInt32)obj->value)",
	"Sign" => "value == 0 ? SmileUnboxedInteger16_From(0) : value > 0 ? SmileUnboxedInteger16_From(1) : SmileUnboxedInteger16_From(-1)",
	"ToChar" => "SmileUnboxedChar_From((Byte)argv[0].unboxed.i16)",
	"ToUni" => "SmileUnboxedUni_From(argv[0].unboxed.i16)"
);

OutputTemplate(\@integerTemplate, \%int16Substitutions, "smileinteger16.generated.c");
OutputTemplate(\@integerBaseTemplate, \%int16Substitutions, "smileinteger16_base.generated.c");

#--------------------------------------------------------------------------------------------

%byteSubstitutions = (
	"type" => "byte",
	"Type" => "Byte",
	"TYPE" => "BYTE",
	"RawType" => "Byte",
	"URawType" => "Byte",
	"unboxed" => "i8",
	"numbits" => "8",
	"TypeName" => "A Byte",
	"ToInt" => "SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.i8)",
	"ToInt64" => "SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.i8)",
	"ToInt32" => "SmileUnboxedInteger32_From((Int32)(UInt32)argv[0].unboxed.i8)",
	"ToInt16" => "SmileUnboxedInteger16_From((Int16)(UInt16)argv[0].unboxed.i8)",
	"ToByte" => "argv[0]",
	"SignExtend64" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.i8)",
	"SignExtend32" => "SmileUnboxedInteger32_From((Int32)argv[0].unboxed.i8)",
	"SignExtend16" => "SmileUnboxedInteger16_From((Int16)argv[0].unboxed.i8)",
	"SignExtend8" =>  "argv[0]",
	"ZeroExtend64" => "SmileUnboxedInteger64_From((Int64)(UInt64)argv[0].unboxed.i8)",
	"ZeroExtend32" => "SmileUnboxedInteger32_From((Int32)(UInt32)argv[0].unboxed.i8)",
	"ZeroExtend16" => "SmileUnboxedInteger16_From((Int16)(UInt16)argv[0].unboxed.i8)",
	"ZeroExtend8" =>  "argv[0]",
	"ToStringBase10" => "String_CreateFromInteger(unboxedData.i8, 10, False)",
	"HashAlgorithm" => "SMILE_APPLY_HASH_ORACLE((UInt32)obj->value)",
	"Sign" => "value == 0 ? SmileUnboxedByte_From(0) : SmileUnboxedByte_From(1)",
	"ToChar" => "SmileUnboxedChar_From(argv[0].unboxed.i8)",
	"ToUni" => "SmileUnboxedUni_From(argv[0].unboxed.i8)"
);

OutputTemplate(\@integerTemplate, \%byteSubstitutions, "smilebyte.generated.c");
OutputTemplate(\@integerBaseTemplate, \%byteSubstitutions, "smilebyte_base.generated.c");
