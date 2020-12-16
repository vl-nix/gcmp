### [Gcmp](https://github.com/vl-nix/gcmp)

#### Calculator

* Multi-Precision arithmetic ( MPFR )

#### Percent

* Example: 1250 % 4 = 1250 * 4 / 100 = 50

#### Dependencies

* libmpfr ( & dev )
* libgtk 3.0 ( & dev )

#### Build

1. Configure: meson build --prefix /usr --strip

2. Build: ninja -C build

3. Install: sudo ninja -C build install

4. Uninstall: sudo ninja -C build uninstall

5. Debug: GCMP_DEBUG=1 gcmp
