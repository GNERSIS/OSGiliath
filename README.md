# OSGiliath

OpenSceneGraph for OpenGL 4.6

## Build

```
cmake --preset default        # clang + Ninja (also: gcc, iwyu)
cmake --build build
```

## Run

```
export OSG_FILE_PATH=/path/to/data
export OSG_LIBRARY_PATH=$PWD/build/lib    # plugin resolution from the build tree
./build/bin/osgviewer model.gltf
```

## Tests

```
cmake --preset default -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## CI

GitLab pipeline (`.gitlab-ci.yml`) on a self-hosted runner. Every job is a
thin wrapper over `.gitlab/scripts/ci-run.sh`, which runs identically on a
developer machine:

```
./.gitlab/scripts/ci-run.sh format    # clang-format gate
./.gitlab/scripts/ci-run.sh build     # build + ctest + 0-warning clang-tidy gate
./.gitlab/scripts/ci-run.sh iwyu      # include-what-you-use gate
./.gitlab/scripts/ci-run.sh full      # all of the above
```

The tidy and iwyu gates are hard (0 findings) and currently red — the
cleanup campaign is tracked in the workspace STATUS doc.
