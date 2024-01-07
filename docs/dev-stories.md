# Stories architecture

## Story location

The stories are located on a SD-Card flash memory. The following file system formats are supported by openStoryTeller:

- FAT32
- exFAT

Connect OpenStoryTeller to your computer using a USB cable, a USB-disk will show up on your file explorer.

## Story tree

A typical folder organisation is showed here:

![arch](./images/dev-sdcard-content.png)

At the root of the SD-Card, a special index file named `index.ost` must be present. It contains the list of installed stories as well as other informations about each story.

## Index file format

TLV format.


