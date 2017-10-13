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

@floatTemplate = LoadTemplate("smilefloat.template");
@floatBaseTemplate = LoadTemplate("smilefloat_base.template");

#--------------------------------------------------------------------------------------------

%float64Substitutions = (
	"type" => "float64",
	"Type" => "Float64",
	"TYPE" => "FLOAT64",
	"RawType" => "Float64",
	"unboxed" => "f64",
	"numbits" => "64",
	"TypeName" => "A Float64",
	"ToBool" => "unboxedData.f64 != 0.0",
	"ToBoolArg" => "argv[0].unboxed.f64 != 0.0",
	"ToInt" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.f64)",
	"ToStringBase10" => "Float64_ToStringEx(unboxedData.f64, 0, 0, False)",
	"ToStringArg" => "Float64_ToStringEx(argv[0].unboxed.f64, 0, 0, False)",
	"HashAlgorithm" => "(UInt32)(*(UInt64 *)&obj->value ^ (*(UInt64 *)&obj->value >> 32))",
	"Zero" => "0.0"
);

OutputTemplate(\@floatTemplate, \%float64Substitutions, "smilefloat64.generated.c");
OutputTemplate(\@floatBaseTemplate, \%float64Substitutions, "smilefloat64_base.generated.c");

#--------------------------------------------------------------------------------------------

%float32Substitutions = (
	"type" => "float32",
	"Type" => "Float32",
	"TYPE" => "FLOAT32",
	"RawType" => "Float32",
	"unboxed" => "f32",
	"numbits" => "32",
	"TypeName" => "A Float32",
	"ToBool" => "unboxedData.f32 != 0.0f",
	"ToBoolArg" => "argv[0].unboxed.f32 != 0.0f",
	"ToInt" => "SmileUnboxedInteger64_From((Int64)argv[0].unboxed.f32)",
	"ToStringBase10" => "Float64_ToStringEx((Float64)unboxedData.f32, 0, 0, False)",
	"ToStringArg" => "Float64_ToStringEx((Float64)argv[0].unboxed.f32, 0, 0, False)",
	"HashAlgorithm" => "(UInt32)(*(UInt32 *)&obj->value)",
	"Zero" => "0.0f"
);

OutputTemplate(\@floatTemplate, \%float32Substitutions, "smilefloat32.generated.c");
OutputTemplate(\@floatBaseTemplate, \%float32Substitutions, "smilefloat32_base.generated.c");

