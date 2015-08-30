#!/usr/bin/perl

@testfiles = ();
sub findfiles
{
	my $path = shift;
	opendir(my $dh, $path) || die "Can't opendir: $!";
	while (my $filename = readdir($dh)) {
		if (($filename =~ m/_tests\.c$/i) && -f "$path/$filename") {
			push @testfiles, "$path/$filename";
		}
		elsif (!($filename =~ m/(^bin$)|(^obj$)|(^\.)/i) && -d "$path/$filename") {
			findfiles("$path/$filename");
		}
	}
	closedir $dh;
}

findfiles(".");

print "Generating test suite include files.\r\n";

@testsuites = ();
foreach $filename (@testfiles) {
	
	open SOURCE, "<$filename";
	my @lines = <SOURCE>;
	close SOURCE;

	$outputname = $filename;
	$outputname =~ s/_tests\.c$/_tests.generated.inc/i;

	open DEST, ">$outputname";
	print DEST "// This file was auto-generated.  Do not edit!\r\n";

	my $last_suite = 0;
	foreach $line (@lines) {
		if ($line =~ m/^\s*TEST_SUITE\((\w+)\)/i) {
			$name = $1;
			if ($last_suite) {
				print DEST "}\r\n" .
					"END_TEST_SUITE($last_suite)\r\n";
			}
			print DEST "\r\nSTART_TEST_SUITE($name)\r\n" .
				"{\r\n";
			$last_suite = $name;
			push @testsuites, $name;
		}
		elsif ($line =~ m/^\s*START_TEST\((\w+)\)/i) {
			$test = $1;
			print DEST "\t$test,\r\n";
		}
	}

	if ($last_suite) {
		print DEST "}\r\n" .
			"END_TEST_SUITE($last_suite)\r\n";
	}

	print DEST "\r\n";
	close DEST;
}

print "Generating testsuites.generated.inc.\r\n";

open DEST, ">testsuites.generated.inc";
print DEST "// This file was auto-generated.  Do not edit!\r\n\r\n";

@testsuites = sort @testsuites;

foreach $testsuite (@testsuites) {
	print DEST "EXTERN_TEST_SUITE($testsuite);\r\n";
}

print DEST "\r\n"
			. "void RunAllTests()\r\n"
			. "{\r\n"
			. "\tTestSuiteResults *results = CreateEmptyTestSuiteResults();\r\n"
			. "\r\n";

foreach $testsuite (@testsuites) {
	print DEST "\tRUN_TEST_SUITE(results, $testsuite);\r\n";
}

print DEST "\r\n"
			. "\tDisplayTestSuiteResults(results);\r\n"
			. "}\r\n"
			. "\r\n";

close DEST;
