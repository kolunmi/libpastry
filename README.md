# libpastry

A collection of Frutiger-Aero-esque widgets and styles for gtk4

## Building

libpastry requires gtk4 as its only dependency

Ensure [git-lfs](https://git-lfs.com/) is installed for your user before cloning:
```sh
brew install git-lfs # or equivalent
git-lfs install
```

Once you've cloned the repository, cd into the new directory and run:
```sh
meson setup build
ninja -C build
```

To run the demo:
```sh
./build/demo/pastry-demo
```
