#!/usr/bin/perl

use Digest::MD5 qw(md5 md5_hex md5_base64);

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

print "Updating test suite include files.\r\n";

$updated = 0;
@testsuites = ();
foreach $filename (@testfiles) {
	
	# Figure out a suitable output filename.
	$outputname = $filename;
	$outputname =~ s/_tests\.c$/_tests.generated.inc/i;

	# Load the source file.
	open SOURCE, "<$filename";
	my @lines = <SOURCE>;
	close SOURCE;

	# Compute a hash of the source file.
	$currentHash = md5_hex(join("\n", @lines));

	# Read the first few lines of the output file to find
	# the source hash, if there is one.
	$lastHash = "";
	if (open GEN, "<$outputname") {
		while (<GEN>) {
			if (m/^\/\/\s*SourceHash:\s*([a-zA-Z0-9]+)/i) {
				$lastHash = $1;
				last;
			}
			if (m/^[^\/]/) {
				# Got a line that's not a comment, so we're
				# not in the comment header anymore and can
				# skip the rest of the file.
				last;
			}
		}
		close GEN;
	}

	# Record the names of any test suites we find, since the summary
	# file will need all of them even if this set didn't change.
	foreach $line (@lines) {
		if ($line =~ m/^\s*TEST_SUITE\((\w+)\)/i) {
			$name = $1;
			push @testsuites, $name;
		}
	}

	# If the source file hasn't changed, don't bother updating the
	# output file.
	if ($lastHash eq $currentHash) {
		next;
	}

	# Generate a new output file.
	print "Writing $outputname\r\n";

	open DEST, ">$outputname";
	print DEST "// This file was auto-generated.  Do not edit!\r\n";
	print DEST "//\r\n";
	print DEST "// SourceHash: $currentHash\r\n";

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

	$updated++;
}

print "$updated files changed.\r\n";

if ($updated > 0) {
	print "Writing testsuites.generated.inc.\r\n";

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

	print DEST "\r\n"
		. "const char *TestSuiteNames[] = {\r\n";

	foreach $testsuite (@testsuites) {
		print DEST "\t\"$testsuite\",\r\n";
	}

	print DEST "};\r\n"
		. "\r\n";

	print DEST "\r\n"
		. "int NumTestSuites = " . @testsuites . ";\r\n"
		. "\r\n";

	close DEST;
}

