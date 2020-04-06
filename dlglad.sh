#!/usr/bin/env zsh
zmodload zsh/pcre

# Generate GL loader and download it
baseurl="https://glad.dav1d.de"

# Download glad.c and glad.h from $baseurl
downloadsource() {
  pcre_compile -s '<a\s+href="(.+?)"'
  strtomatch="glad/$3\">$3"
  pcre_match -a urls "$(grep -F $strtomatch $1)"

  print "getting ${baseurl}${urls[1]}" 
  if [[ $2 == "wget" ]]; then
    $2 -O $3 "${baseurl}${urls[1]}"
  elif [[ $2 == "curl" ]]; then
    $2 "${baseurl}${urls[1]}" > $3
  fi
}

# Download the HTML file containing the links to the generated GLAD files
downloadtemplate() {
  rqprog="wget"
  if ! which $rqprog >/dev/null; then
    rqprog="curl"
  fi
  if ! which $rqprog >/dev/null; then
    exit 1 # curl not found
  fi

  if [[ $rqprog == "wget" ]]; then
    $rqprog --post-data=$postdata -O $1 "${baseurl}/generate"
  elif [[ $rqprog == "curl" ]]; then
    $rqprog -L --data $postdata "${baseurl}/generate" > $1
  fi

  print $rqprog
}

postdata="language=c&\
specification=gl&\
api=gl%3D3.3&\
api=gles1%3Dnone&\
api=gles2%3Dnone&\
api=glsc2%3Dnone&\
profile=core&\
loader=on&\
localfiles=on"
templatefname="glad.tmp.html"

rqprog="$(downloadtemplate $templatefname)"
downloadsource $templatefname $rqprog glad.h
downloadsource $templatefname $rqprog glad.c
downloadsource $templatefname $rqprog khrplatform.h

#mkdir -p vendor/glad
#mv -t vendor/glad glad.c glad.h khrplatform.h

rm $templatefname