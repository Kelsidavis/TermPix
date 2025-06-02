{
  description = "TermPix - High-resolution terminal image renderer";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        
        termpix = pkgs.stdenv.mkDerivation rec {
          pname = "termpix";
          version = "2.0";
          
          src = ./.;
          
          buildInputs = with pkgs; [
            # Math library is linked automatically by stdenv
          ];
          
          nativeBuildInputs = with pkgs; [
            gcc
            gnumake
          ];
          
          buildPhase = ''
            make
          '';
          
          installPhase = ''
            mkdir -p $out/bin
            cp termpix $out/bin/
          '';
          
          meta = with pkgs.lib; {
            description = "High-resolution terminal image renderer";
            homepage = "https://github.com/jamesbrink/TermPix";
            license = licenses.mit;
            maintainers = [ ];
            platforms = platforms.all;
          };
        };
      in
      {
        packages = {
          default = termpix;
          termpix = termpix;
        };
        
        apps = {
          default = flake-utils.lib.mkApp {
            drv = termpix;
          };
          termpix = flake-utils.lib.mkApp {
            drv = termpix;
          };
        };
        
        devShells.default = pkgs.mkShell {
          inputsFrom = [ termpix ];
          
          buildInputs = with pkgs; [
            # Build tools
            gcc
            gnumake
            cmake
            pkg-config
            
            # Development tools
            gdb
            # valgrind # Not available on macOS
            clang-tools
            bear
            
            # Code formatting and linting
            clang-tools
            cppcheck
            
            # Documentation
            doxygen
            graphviz
            
            # Testing tools
            python3
            imagemagick
            
            # Version control
            git
            
            # Editor support
            ccls
            
            # Windows cross-compilation (optional)
            pkgsCross.mingwW64.stdenv.cc
          ];
          
          shellHook = ''
            echo "TermPix Development Environment"
            echo "==============================="
            echo ""
            echo "Available commands:"
            echo "  make              - Build termpix"
            echo "  make clean        - Clean build artifacts"
            echo "  ./termpix <image> - Run termpix"
            echo ""
            echo "Development tools:"
            echo "  gdb               - Debugger"
            echo "  bear              - Compilation database generator"
            echo "  cppcheck          - Static analyzer"
            echo "  clang-format      - Code formatter"
            echo ""
            echo "Cross-compilation:"
            echo "  x86_64-w64-mingw32-gcc - Windows cross-compiler"
            echo ""
            
            # Set up development environment
            export CFLAGS="-Wall -Wextra -pedantic -std=c99"
            export CC="${pkgs.gcc}/bin/gcc"
            
            # Create compile_commands.json for better IDE support
            if [ ! -f compile_commands.json ]; then
              echo "Generating compile_commands.json..."
              make clean > /dev/null 2>&1
              bear -- make > /dev/null 2>&1
              make clean > /dev/null 2>&1
            fi
          '';
        };
      });
}