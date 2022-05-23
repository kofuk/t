#!/usr/bin/env bash
mkdir data
dd if=/dev/zero of=hoge.img bs=1G count=2
mkfs.btrfs hoge.img
# discard=async is not supported on Ubuntu?
sudo mount -t btrfs -o loop,nodatacow hoge.img data/
#sudo mount -t btrfs -o loop,nodatacow,discard=async hoge.img data/

cd data
btrfs subvolume snapshot . s0
sudo btrfs subvolume s0
