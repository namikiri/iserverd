#!/usr/bin/perl
use File::Temp;

$ob_location  = "/paging/paging_ob.html"; # Web-page to show on errors
$ok_location  = "/paging/paging_ok.html"; # Web-page to show on success

$webpager_bin = "/usr/bin/webpager";
$tmp_path     = "/tmp";

$ip= $ENV{'REMOTE_ADDR'};
$lng = $ENV{'CONTENT_LENGTH'};

# read parameters
read(STDIN, $buff, $ENV{'CONTENT_LENGTH'});

# paranoid parameters clean and check
@pairs = split(/&/, $buff);
%FORM = "";
foreach $pair (@pairs) {
   ($name, $value) = split(/=/, $pair);
   
   # Un-Webify plus signs and %-encoding
   $value =~ tr/+/ /;
   $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
   $value =~ s/<!--(.|\n)*-->//g;
   
   if ($allow_html != 1) {
         $value =~ s/<([^>]|\n)*>//g;
   }
   
   $FORM{$name} = $value;
}

# we need _REAL_ addres
if ($ENV{'HTTP_X_FORWARDED_FOR'}){
    $FORM{ip} = $ENV{'HTTP_X_FORWARDED_FOR'};
    } else { $FORM{ip} = $ENV{'REMOTE_ADDR'}; }

# webpager will delete this temp file after reading
($fh,$fname) = mkstemp("$tmp_path/tempXXXXXX");
open (FILE, ">$fname");
print FILE "$FORM{uin}\n$FORM{email}\n$FORM{name}\n$FORM{ip}\n$FORM{msg}";
# print $fname;

close(FILE);

# execute webpager utility to send message
$out=`$webpager_bin $tmp_path/$fname`;
print $d;
if (!$out){
print "Location: $ob_location\n\n";
} else {
print "Location: $ok_location\n\n";
} 
