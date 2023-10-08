#!/usr/bin/awk -f
BEGIN {ix=-1;longestbit=-1;}
$1 ~ /^#/ {  next; }			# remove comments
/^[[:space:]]*$/ { next; }
$1=="-" {
  bit=ix","bits[ix];
  bitname[bit]=$2;
  bitvals[bit]=0;
  if (longestbit<length($2)) longestbit=length($2);
  if ($3 ~ /[.:,-]/ || $4) { // bit range
    s=$3"-"$4$5;
    gsub(/[.:,-]+/, ",", s);
    split(s, a, ",");
    if (a[1]>a[2]) { b=a[2]; bb=a[1]-a[2]+1;} else
    if (a[1]<a[2]) { b=a[3]; bb=a[2]-a[1]+1;} else
                   { b=a[1]; bb=1;}
  } else { b=$3; bb=1; }
  bitbit[bit]=b;
  bitbits[bit]=bb;
  bits[ix]++;
  next;
}
$1=="=" {
  bitval=bit","bitvals[bit]; bitvals[bit]++;
  bvname[bitval]=bitname[bit] $2;
  bvval[bitval]=$3;
  if (bitvalslen[bit]<length(bvname[bitval])) bitvalslen[bit]=length(bvname[bitval]);
  next;
}
{ 
  ix++;
  bits[ix]=0;
  rid[ix]="0x"$1; 
  rname[ix]=$2; 
}
END {
  ix++;
  UREG=toupper(REG);
  print "enum REG" UREG " {";
  for(i=0;i<ix;i++) print "/*"i"*/\t" rname[i] "\t = \t " rid[i] ","
  print "};"
  
  m=1;for(i=0; i<8; i++) m+=mask[i]=m;	// powers of 2
  for(i=0;i<ix;i++) {
    if (!bits[i]) continue;
    printf "// %s(%d) %s\n", rid[i], rid[i], rname[i];
    for(j=0;j<bits[i];j++) {
      bit=i","j;
      printf "#define BIT_%-"longestbit"s %d\n", toupper(bitname[bit]), bitbit[bit];
    }
    for(j=0;j<bits[i];j++) {
      bit=i","j;
      m=0; for(k=bitbit[bit]; k<bitbit[bit]+bitbits[bit]; k++) m+=mask[k];
      printf "#define %-"longestbit"s 0x%02X\n", toupper(bitname[bit]), m;
      for(k=0;k<bitvals[bit];k++) {
        bv=bit","k;
        printf "#define %-"bitvalslen[bit]"s 0x%02X\n", toupper(bvname[bv]), bvval[bv]*mask[bitbit[bit]];
      }
    }    
  }
  
#  print "#ifdef DBG_REGS"
  print "#include <map>"
  print "#include <string>"
  print "#include <sstream>"
  print "std::map<int,std::string> reg" UREG "name = {"
  for(i=0;i<ix;i++) print "\t{" rid[i] ", \"" rname[i] "\"},"
  print "};"
  
  print "std::string reg"UREG"DataString(int reg, int data) {"
  print "  std::ostringstream ss;"
  print "  ss << \"0x\" << std::uppercase << std::hex << data;"
  print "  return ss.str();"
  print "}"
  
#  print "#endif"
}
