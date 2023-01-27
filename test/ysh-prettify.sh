#!/usr/bin/env bash
#
# Test ysh-prettify transformations
#
# Usage:
#   ./ysh-prettify.sh <function name>

set -o nounset
set -o pipefail
set -o errexit
shopt -s strict:all 2>/dev/null || true  # dogfood for OSH

source test/common.sh

readonly TEMP_DIR=_tmp

check-osh2ysh() {
  local osh_str=$1
  local ysh_str=$2  # expected
  local allow_invalid=${3:-}

  # Make sure they are valid

  bin/osh -n -c "$osh_str"
  if test -z "$allow_invalid"; then
    bin/ysh -n -c "$ysh_str"
  fi

  local tmp=$TEMP_DIR/actual.ysh
  echo "$osh_str" | bin/oshc translate | tee $tmp

  echo "$ysh_str" | diff -u $tmp -
  echo 'OK'
}

#
# UNCHANGED
#

test-simple-command() {
  ### Unchanged

  check-osh2ysh 'echo hi' 'echo hi'
}


test-line-breaks() {
  ### Unchanged

  check-osh2ysh '
echo one \
  two three \
  four
' '
echo one \
  two three \
  four
'
}

test-and-or() {
  check-osh2ysh \
    'ls && echo "$@" || die "foo"' \
    'ls && echo @ARGV || die "foo"'
}

#
# CHANGED WORD LANGUAGE
#

test-dollar-at() {
  check-osh2ysh \
    'echo one "$@" two' \
    'echo one @ARGV two'
}

TODO-test-prefix-ops() {
  check-osh2ysh \
    'echo ${#s} ${#a[@]}' \
    'echo $[len(s)] $[len(a)]'
}

test-unquote-subs-TODO() {
  check-osh2ysh \
    'echo "$1" "$foo"' \
    'echo $1 $foo'

  # TODO: $foo and $(echo hi)

  check-osh2ysh \
    'echo "${foo}"' \
    'echo $(foo)' \

  check-osh2ysh \
    'echo "$(echo hi)"' \
    'echo $[echo hi]' \
    INVALID
}

# TODO: translate to YSTR?

TODO-test-word-joining() {
  local osh=$(cat <<EOF
echo 'foo " bar '"'" 
EOF
)
  local ysh=$(cat <<EOF
echo y"foo \" bar '"
EOF
)
  check-osh2ysh "$osh" "$ysh"
}

test-command-sub-TODO() {
  check-osh2ysh \
    'echo $(echo hi)' \
    'echo $[echo hi]' \
    INVALID

  check-osh2ysh \
    'echo "__$(echo hi)__"' \
    'echo "__$[echo hi]__"' \
    INVALID
}

test-var-sub-TODO() {
  check-osh2ysh \
    'echo $foo' \
    'echo $foo'

  check-osh2ysh \
    'echo $foo ${bar} "__${bar}__"' \
    'echo $foo $(bar) "__$(bar)__"'

  return

  # We could make this $[foo ? 'default'], but meh, let's not introduce more
  # operators

  check-osh2ysh \
    'echo ${foo:-default}' \
    "echo \$(foo or 'default')"
}

# Downgraded to one_pass_parse.  This means \" will be wrong, but meh.
# Here the WordParser makes another pass with CommandParser.
#
# We could also translate it to:
#   echo $[compat backticks 'echo hi']
# But that might be overly pedantic.  This will work most of the time.

test-backticks-TODO() {

  # Make this pass
  check-osh2ysh \
   'echo `echo hi ${var}`' \
   'echo $[echo hi $(var)]' \
   INVALID

  # These also have problems
  check-osh2ysh \
    'echo `{ echo hi; }`' \
    'echo $[do { echo hi]' \
    INVALID

  check-osh2ysh \
    'echo $({ echo hi; })' \
    'echo $[do { echo hi]' \
    INVALID
}

#
# CHANGED BUILTIN LANGUAGE
#

test-bracket-builtin() {
  check-osh2ysh \
    '[ ! -z "$foo" ] || die' \
    'test ! -z $foo || die'

  check-osh2ysh '
if [ "$foo" -eq 3 ]; then
  echo yes
fi' \
  '
if test $foo -eq 3 {
  echo yes
}'
}

test-source-builtin() {
  check-osh2ysh \
    '. lib.sh' \
    'source lib.sh'

  check-osh2ysh \
    '[ -f lib.sh ] && . lib.sh' \
    'test -f lib.sh && source lib.sh'
}

TODO-test-set-builtin() {
  # Not needed now that we have 'setvar' ?

  check-osh2ysh \
    'set -o errexit' \
    'shopt --set errexit'
}

# 
# CHANGED COMMAND LANGUAGE
#

test-while-loop() {
  check-osh2ysh '
while read line; do
  echo $line
done' \
  '
while read line {
  echo $line
}'

  check-osh2ysh '
while read \
  line; do
  echo $line
done' \
  '
while read \
  line {
  echo $line
}'
}

test-if() {
  check-osh2ysh '
if true; then
  echo yes
fi' \
  '
if true {
  echo yes
}'

  check-osh2ysh '
if true; then
  echo yes
elif false; then
  echo elif
elif spam; then
  echo elif
else
  echo no
fi' \
  '
if true {
  echo yes
} elif false {
  echo elif
} elif spam {
  echo elif
} else {
  echo no
}'
}

# TODO: Fix this

TODO-test-then-next-line() {
  check-osh2ysh '
if true
then
  echo yes
fi' \
  '
if true {
  echo yes
}'

}

test-posix-func() {
  check-osh2ysh '
  f() {
    echo "hi"
  }' '
  proc f {
    echo "hi"
  }'

  return

  # TODO: Move the brace
  check-osh2ysh '
  f()
  {
    echo "hi"
  }' '
  proc f
  {
    echo "hi"
  }'

  # No nested functinos
  return
  check-osh2ysh '
func1() {
  echo func1
  func2()
  {
    echo func2
  }
}' \
  '
proc func1 {
  echo func1
  proc func2
  {
    echo func2
  }
}'

}

test-ksh-func() {
  check-osh2ysh '
function func1 {  # no parens
  echo func1
}' '
proc func1 {  # no parens
  echo func1
}'
}

test-for-loop() {
  check-osh2ysh '
for x in a b c \
  d e f; do
  echo $x
done
' '
for x in [a b c \
  d e f] {
  echo $x
}
'

  check-osh2ysh '
for x in a b c \
  d e f
do
  echo $x
done
' '
for x in [a b c \
  d e f]
{
  echo $x
}
'
}

test-empty-for-loop() {
  check-osh2ysh '
for x in 
do
  echo $x
done
' '
for x in []
{
  echo $x
}
'
}

test-args-for-loop() {
  # Why are we missing a newline here?
  check-osh2ysh '
for x; do
  echo $x
done
' 'for x in @ARGV {
  echo $x
}
'
  # Change brace style

  check-osh2ysh '
for x
do
  echo $x
done
' 'for x in @ARGV {
  echo $x
}
'
}

# TODO: Fix this!

test-subshell() {
  check-osh2ysh \
    '(echo hi;)' \
    'shell {echo hi;}' \
    INVALID

  check-osh2ysh \
    '(echo hi)' \
    'shell {echo hi}' \
    INVALID

  check-osh2ysh \
    '(echo hi; echo bye)' \
    'shell {echo hi; echo bye}' \
    INVALID

  check-osh2ysh \
    '( (echo hi; echo bye ) )' \
    'shell { shell {echo hi; echo bye } }' \
    INVALID
}

test-brace-group() {
  check-osh2ysh \
    '{ echo hi; }' \
    'do { echo hi; }' \
    INVALID

  check-osh2ysh \
    '{ echo hi; echo bye; }' \
    'do { echo hi; echo bye; }' \
    INVALID
}

# TODO: Change case syntax

test-case() {
  check-osh2ysh '
case $var in
  foo|bar)
    [ -f foo ] && echo file
    ;;
  "")
    echo empty
    ;;
  *)
    echo default
    ;;
esac
' '
match $var {
  with foo|bar
    test -f foo && echo file
    
  with ""
    echo empty
    
  with *
    echo default
    
}
'

  check-osh2ysh '
case "$var" in
  *)
    echo foo
    echo bar  # no dsemi
esac
' '
match $var {
  with *
    echo foo
    echo bar  # no dsemi
}
'
}

soil-run() {
  run-test-funcs
}

"$@"
