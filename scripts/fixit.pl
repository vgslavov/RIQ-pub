#!/usr/bin/perl

# by http://people.apache.org/~andy/Freebase20121223

$line = 0 ;
while(<>)
{
    $line++ ;
    if ( /^\s*$/ )
    {
	print ;
	next ;
    }
    if ( /^\@/ )
    {
	# Directive
	print ;
	next ;
    }
    # Fixup broken lines - all of the form of raw newline at the end of a literal
    # with the next line being ".
    if ( ! /\.$/ )
    {
	$X = $_ ;
	chomp($X) ;
	$Y = <> ;
	$line++ ;
	$_=$X.$Y ;
    }

    # Parse line into subject/predicate/rest.
    if ( ! /^(\S+)\s+(\S+)\s+(.*)$/ )
    {
	print STDERR "ERROR: $line: Failed to parse the line\n" ;
	print STDERR $_ ;
	next ;
    }

    $subj = $1 ;
    $pred = $2 ;
    $obj = $3 ; # Remainder, to be sorted out.

    # End in DOT?
    if ( $obj !~ /\.$/ )
    {
	print STDERR "ERROR: $line: $obj\n" ;
	print STDERR $_ ;
	next ;
    }
    # Remove leading white space, DOT and newline
    chomp($obj) ;
    $obj =~ s/^\s*// ;
    $obj =~ s/\s*\.$// ;

    # No object.
    if ( $obj eq '' )
    {
	next if ( $pred eq 'ns:base.motorsports.circuit_layout.layout_length' ) ;
	print STDERR "ERROR: $line: Unexpected empty object\n" ;
	print STDERR $_ ;
    }

    next if ( $obj eq '' ) ;

    # Fixups.
    # Some prefixed names $ is used as a unicode escape (?)
    # \$ is legal in RDF 1.1/Turtle.
    # Alt: convert to %xx%xx.

    # vsfgd: don't do that, Jena TDB 2.3.11 errors out
    # Subject - always a prefix name
    #$subj =~ s/\$/\\\$/g ;
    # Predicate - always a prefix name
    #$pred =~ s/\$/\\\$/g ;

    # Object - various
    # Fix up URIs (only found in object position)
    if ( $obj =~ m/^<(.*)>$/ )
    {
	$X = $1 ;
	$X =~ s/</%3C/g ;
	$X =~ s/\\>/%3E/g ;
	$X =~ s/\\.//g ;
 	$obj = "<".$X.">" ;
    }
    elsif ( $obj =~ /^"/ )
    {
	# Literal
    }
    # vsfgd: don't do that, Jena TDB 2.3.11 errors out
    #else # prefixed name
    #{
	#$obj =~ s/\$/\\\$/g ;
    #}

    print "$subj $pred $obj .\n" ;
}
