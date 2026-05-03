{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    systems.url = "github:nix-systems/default";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };
  outputs =
    inputs@{ flake-parts, systems, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } (
      { ... }:
      {
        systems = import systems;
        perSystem =
          { pkgs, ... }:
          {
            devShells.default =
              pkgs.mkShell.override
                {
                  stdenv =
                    if pkgs.stdenv.isLinux then
                      pkgs.stdenvAdapters.useMoldLinker pkgs.llvmPackages.libcxxStdenv
                    else
                      pkgs.llvmPackages.libcxxStdenv;
                }
                rec {
                  packages = with pkgs; [
                    nil
                    nixd
                    nixfmt-tree
                    nixfmt
                    mold
                    cmake
                    ninja
                    pkg-config
                    llvmPackages.clang-tools
                    llvmPackages.lldb
                  ];
                  buildInputs = with pkgs; [
                    libcxx
                    glfw
                    libxkbcommon
                    wayland-scanner
                    wayland
                    libx11
                    libxcursor
                    libxi
                    libxrandr
                    libxcb
                    libxinerama
                    udev
                    mbedtls
                    libssh2
                    freetype
                    harfbuzz
                    libvorbis
                    libogg
                    flac
                  ];
                  LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath buildInputs;
                };
            formatter = pkgs.nixfmt-tree;
          };
      }
    );
}
