==================
MiracleWhiteKnight
==================

What is MiracleWhiteKnight
==========================

This is a a project which provides mandatory access control based on allow list and protects the list in Trusted Execution Environment.

Requirements
============

.. list-table::
   :header-rows: 1

   - * Name
     * When
     * Version
   - * openssl
     * build & runtime
     * 1.1.1c or later
   - * meson
     * build
     * 0.49 or later
   - * ninja
     * build
     *
   - * pkg-config
     * build
     *
   - * gcc
     * build
     *

How to install
==============

#. Build binaries.

   .. code-block:: fish

      $ meson build
      $ ninja -C build

#. Install the built binaries.

   .. code-block:: fish

      $ ninja -C build install

How to use
==========

MiracleWhiteKnight provides 2 management methods for allow list.

1. Do not use TEE
2. Use TEE

Do not use TEE
--------------

#. Make a list of directories to search executables

   .. code-block:: fish

      $ echo '/usr' > in.txt

#. Generate allow list of your root filesystem

   .. code-block:: fish

      # mkag rootfs.wl in.txt
      # mv rootfs.wl /usr/share/whiteknight/wl.d/

#. Generate a list of mount points

   .. code-block:: fish

      # echo '/' > /usr/share/whiteknight/ml.d/rootfs.ml

#. Run MiracleWhiteKnight

   .. code-block:: fish

      # systemctl start whiteknight

Use TEE
-------

Coming soon...


Contribution
============

Please send PRs via GitHub. We are waiting for your contribution anytime!
