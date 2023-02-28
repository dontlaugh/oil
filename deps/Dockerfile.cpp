FROM oilshell/soil-common

# Copy again to prevent unsound caching
COPY deps/from-apt.sh /home/uke/tmp/deps/from-apt.sh

RUN --mount=type=cache,id=var-cache-apt,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=var-lib-apt,target=/var/lib/apt,sharing=locked \
    du --si -s /var/cache/apt /var/lib/apt && \
    deps/from-apt.sh cpp

# Build other dependencies as non-root uke
USER uke

# Copy pre-built wedges

COPY --chown=uke \
  _build/wedge/binary/oils-for-unix.org/pkg/cmark/0.29.0 \
  /wedge/oils-for-unix.org/pkg/cmark/0.29.0

COPY --chown=uke \
  _build/wedge/binary/oils-for-unix.org/pkg/re2c/3.0 \
  /wedge/oils-for-unix.org/pkg/re2c/3.0

# For measuring binary size
COPY --chown=uke \
  _build/wedge/binary/oils-for-unix.org/pkg/bloaty/1.1 \
  /wedge/oils-for-unix.org/pkg/bloaty/1.1

COPY --chown=uke \
  _build/wedge/binary/oils-for-unix.org/pkg/python3/3.10.4 \
  /wedge/oils-for-unix.org/pkg/python3/3.10.4

# TODO:
# - add uftrace for benchmarks
# - add osh-runtime testdata
# - not running ovm-build yet, but it also has testdata

COPY deps/from-R.sh /home/uke/tmp/deps/from-R.sh
RUN deps/from-R.sh other-tests

# pexpect will use the python3 wedge; it can also use system Python

COPY build/dev-shell.sh /home/uke/tmp/build/dev-shell.sh
COPY deps/from-py.sh /home/uke/tmp/deps/from-py.sh

RUN deps/from-py.sh cpp

# Install mycpp deps with python3 wedge
COPY mycpp/common-vars.sh /home/uke/tmp/mycpp/common-vars.sh
COPY deps/from-git.sh /home/uke/tmp/deps/from-git.sh

RUN deps/from-git.sh layer-mycpp

CMD ["sh", "-c", "echo 'hello from oilshell/soil-cpp'"]
