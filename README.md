# Character_Device
Simple character devices read-write , ioctl and poll implemented as LKMs(5.15.0-58-generic kernel version)


⬇️ To try this out ⬇️

  1. Clone this repo on your linux machine.
  2. Extract the file
  3. Open the file that you want to execute (like poll or read_write).
  4. Open the terminal where you extract the file and make root (with sudo su).
  5. make
  6. insmod char_dev.ko
  7. dmesg (to see if device created, and also see major and minor numbers).
  8. ./main (run the main file to test character device)    
  9. If permission denied to open or read or write etc. You can run "mknod /dev/char_dev c 508 0" (c: character device, 508: your major number, 0: your minor number). If you want to remove the device file you can run  "rm /dev/char_dev"
  10. You can exit the device " rmmod /dev/char_dev    

I suggest you "root" before running these files. If don't, you can run the above commands by adding "sudo" at the beginning.
