{
  description = "Nero-UMU — an umu-launcher manager and launcher";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    quazip-src = {
      url = "github:stachenov/quazip";
      flake = false;
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      quazip-src,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };

        nero-umu = pkgs.callPackage ./default.nix { inherit quazip-src; };
      in
      {
        packages.default = nero-umu;
        packages.nero-umu = nero-umu;

        devShells.default = pkgs.mkShell {
          inputsFrom = [ nero-umu ];
          packages = with pkgs; [
            gdb
            ninja
          ];
        };
      }
    );
}
