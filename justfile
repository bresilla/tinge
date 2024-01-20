just:
	xmake
	xmake project -k compile_commands
	xmake project -k cmake
    xmake package

build:
    xmake

# book:
    mdbook build book --dest-dir ../docs
    git add -A && git commit -m "docs: building website/mdbook"

do type:
    #!/usr/bin/env bash
    new_version=$(semver $(grep -oP 'set_version\("\K[^"]+' xmake.lua) {{type}})
    sed -i "s/\(set_version(\)\"[^\"]*\"/\1\"$new_version\"/" xmake.lua
    git cliff --tag $new_version > CHANGELOG.md
    changelog=$(git cliff --unreleased --strip all)
    git add -A && git commit -m "chore(release): prepare for $new_version"
    git tag -a $new_version -m "$new_version" -m "$changelog"
    git push --follow-tags --force --set-upstream origin develop
    gh release create $new_version --notes "$changelog"