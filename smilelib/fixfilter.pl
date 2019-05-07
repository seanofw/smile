
sub uniq {
	my %seen;
	return grep { !$seen{$_}++ } @_;
}

sub CalculateFilter {
	my $path = shift;
	if ($path =~ m/^include\\smile\\(.*)\\([^\\]+)$/) {
		return "include\\$1";
	}
	elsif ($path =~ m/^include\\smile\\([^\\]+)$/) {
		return "include";
	}
	elsif ($path =~ m/^(.*)\\([^\\]+)$/) {
		return $1;
	}
	else {
		return "";
	}
}

@names = ();
%types = ();

while (<>) {
	if (m/^\s*<(None|Text|ClCompile|ClInclude|Natvis)\s+Include=\"([^\"]*)\"/i) {
		$type = $1;
		$name = $2;
		push @names, $name;
		$types{$name} = $type;
	}
}

@names = sort @names;

@filters = ();
foreach (@names) {
	$filter = CalculateFilter($_);
	push @filters, $filter;
	while ($filter =~ m/^(.*)\\[^\\]+$/) {
		$filter = $1;
		push @filters, $filter;
	}
}
@filters = uniq sort @filters;

print "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
print "<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\r\n";

print "  <ItemGroup>\r\n";
foreach (@filters) {
	if ($_ ne "") {
		print "    <Filter Include=\"$_\" />\r\n";
	}
}
print "  </ItemGroup>\r\n";

print "  <ItemGroup>\r\n";

$isFirst = 1;
$lastfilterprefix = "";
foreach (@names) {
	$name = $_;
	$type = $types{$name};
	$filter = CalculateFilter($name);
	if ($filter =~ m/^([^\\]+)\\?/) {
		$filterprefix = $1;
	}
	else {
		$filterprefix = "";
	}
	if ($filterprefix ne $lastfilterprefix) {
		if ($isFirst != 1) {
			#print "  </ItemGroup>\r\n";
			#print "  <ItemGroup>\r\n";
		}
		$lastfilterprefix = $filterprefix;
	}
	$isFirst = 0;
	if ($filter ne "") {
		print "    <$type Include=\"$name\">\r\n";
		print "      <Filter>$filter</Filter>\r\n";
		print "    </$type>\r\n";
	}
	else {
		print "    <$type Include=\"$name\" />\r\n"
	}
}

print "  </ItemGroup>\r\n";
print "</Project>\r\n";

