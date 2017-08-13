
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

%int64Substitutions = (
	"type" => "integer64",
	"Type" => "Integer64",
	"TYPE" => "INTEGER64",
	"RawType" => "Int64",
	"URawType" => "UInt64",
	"unboxed" => "unboxed.i64",
	"TypeName" => "An Integer64",
	"ToInt32" => "(Int32)(obj->end - obj->start)",
	"ToReal64" => "Real64_FromInt64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"ToStringBase10" => <<EOI,
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
	"ToString" => <<EOI,
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
	"HashAlgorithm" => <<EOI,
		UInt64 start = (UInt64)range->start;
		UInt64 end = (UInt64)range->end;
		UInt64 stepping = (UInt64)range->stepping;
		result = (UInt32)(start ^ (start >> 32)) ^ (UInt32)(end ^ (end >> 32)) ^ (UInt32)(stepping ^ (stepping >> 32));
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
	"ToInt32" => "(obj->end - obj->start)",
	"ToReal64" => "Real64_FromInt64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"ToStringBase10" => <<EOI,
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
	"ToString" => <<EOI,
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
	"HashAlgorithm" => <<EOI,
		UInt32 start = (UInt32)range->start;
		UInt32 end = (UInt32)range->end;
		UInt32 stepping = (UInt32)range->stepping;
		result = (UInt32)(start ^ end ^ stepping);
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
	"ToInt32" => "(obj->end - obj->start)",
	"ToReal64" => "Real64_FromInt64(obj->end - obj->start)",
	"ToFloat64" => "(Float64)(obj->end - obj->start)",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"ToStringBase10" => <<EOI,
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
	"ToString" => <<EOI,
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
	"HashAlgorithm" => <<EOI,
		UInt16 start = (UInt16)range->start;
		UInt16 end = (UInt16)range->end;
		UInt16 stepping = (UInt16)range->stepping;
		result = (UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16));
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
	"ToInt32" => "((Int32)obj->end - (Int32)obj->start)",
	"ToReal64" => "Real64_FromInt64((Int32)obj->end - (Int32)obj->start)",
	"ToFloat64" => "(Float64)((Int32)obj->end - (Int32)obj->start)",
	"length" => "self->end > self->start ? self->end - self->start + 1 : self->start - self->end + 1",
	"ToStringBase10" => <<EOI,
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
	"ToString" => <<EOI,
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
	"HashAlgorithm" => <<EOI,
		Byte start = range->start;
		Byte end = range->end;
		Byte stepping = range->stepping;
		result = (UInt32)((UInt32)start ^ (UInt32)(end << 8) ^ (UInt32)(stepping << 16));
EOI
);

OutputTemplate(\@rangeTemplate, \%byteSubstitutions, "smilebyterange.generated.c");
OutputTemplate(\@rangeBaseTemplate, \%byteSubstitutions, "smilebyterange_base.generated.c");
