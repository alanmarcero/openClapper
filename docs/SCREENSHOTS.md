# Screenshots

## OpenVST Manager preview

OpenVST Manager shows `docs/screenshots/preview.png` from `main` (see `screenshotPath` in its catalog), and the GitHub release body and README hero point at the same file. This is the single image to keep current. Refresh it with the main editor capture whenever the UI changes, and commit it before tagging a release:

```sh
mkdir -p build/shots && OPENCLAPPER_SCREENSHOTS="$PWD/build/shots" build/openclapper_plugin_tests '[plugin][gui]'
cp build/shots/openClapper-screenshot.png docs/screenshots/preview.png
```
