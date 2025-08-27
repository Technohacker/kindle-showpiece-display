# Showpiece Display with a Kindle

![Demo Image for this Project](./doc/demo.jpg)

This project turns a jailbroken Kindle into a simple, low-power showpiece display that regularly pulls images computed on an Arduino over WiFi.

## How to Run
There are two components to this project, the [Display Script](./showpiece_display.sh), and the [Server](./arduino_server/). You can use each component independently, and swap them for different implementations if needed.

The server is written for an [Arduino Nano RP2040 Connect](https://docs.arduino.cc/hardware/nano-rp2040-connect/), and will require changes for other microcontrollers. Open the folder in the Arduino IDE, edit `arduino_secrets.h` to set your WiFi credentials, and flash it onto your board. The board will connect to the network and print out the URL you need to use over Serial.

To use the Display Script, you need a jailbroken Kindle. In my case I have a Kindle Paperwhite PW3 with [WinterBreak](https://github.com/KindleModding/WinterBreak/). The script is designed to run as a [Scriptlet](https://kindlemodding.org/kindle-dev/scriptlets.html): putting it in the `documents` folder of your Kindle (`/mnt/us/documents` from inside) will make it visible as a book called "Showpiece Display" that you can open to launch it. Make sure to edit the script to set the URL you got from the Arduino beforehand.

## More Info
Before we get to the details of this project, I want to preface it with an observation I'd made:

I had the idea for this project when I saw my Kindle and Arduino lying around unused. I figured it'd be a nice little "interactive decor" experiment, since the Kindle could just hold its image when switched off, and the Arduino on its own would sip very little power. What I _didn't_ expect was how, despite the apparent mundane-ness of this project, it presented interesting challenges and solutions that could genuinely carry over as actual development skills.

Often when one studies Programming or Computer Science, they follow a very theory-heavy curriculum. In a sense, this theory-first approach teaches students about the kinds of tools that can be used (for example, Data Structures and Algorithms). However, in the world of CompSci these tools and topics are by definition highly abstract, and it's often hard to see how they'd get used in real applications.

A very common way that I've seen this apparent disconnect get resolved is with practice problems, like the kinds you would often see in Competitive Programming. These are a step up, but they are often very synthetic/artifically built problems made with the express intent of exercising particular topics. This has had the unfortunate side effect of planting a distorted view of CompSci, making it seem as if the mastery of such tools is all you need.

In my opinion, the missing piece is an understanding of _where_ one would apply the tools they were taught about. And from my personal experience, the best way to gain this understanding is to discover it organically as you build projects. Even mundane ones can give you the opportunity to face real challenges, if the constraints are just right.

To that end I say, to anyone who wishes to learn Computer Science: **Make projects!** You could even make something simple, or remake something that already exists. It doesn't matter even if the project seems mundane, you can always add constraints to make it interesting. As long as you find the process fun (and more importantly, you put your heart and soul into it), you'll learn skills that will carry over without you ever realising!

Now, philosophical exposition aside, I'll highlight a few examples of moments in this project that felt interesting to me:

### Running code on the Kindle
For those that have used a Kindle before, you may be surprised to find that it runs on Linux! Naturally, I'm not [the](https://blog.4dcu.be/diy/2020/10/04/PythonKindleDashboard_2.html) [first](https://terminalbytes.com/reviving-kindle-paperwhite-7th-gen/) [person](https://github.com/pascalw/kindle-dash) to use a Kindle as a display, and I borrowed heavily from those before me.

Once you jailbreak your Kindle and get KUAL installed, it's only a matter of installing [kterm](https://github.com/bfabiszewski/kterm) to get access to the terminal on-device, and [USBNetwork](https://wiki.mobileread.com/wiki/USBNetwork) to get SSH access over WiFi. USBNetwork also provides `nano` to edit files, and it's quite the surreal experience to use it on this device! I opted to edit the display script via SSH on the device.

Some resources I've found great for the Kindle are the [KindleModding Wiki](https://kindlemodding.org/) and [MobileRead Wiki](wiki.mobileread.com/wiki/Main_Page)

### Handling images on the Arduino
I had an [Arduino Nano RP2040 Connect](https://docs.arduino.cc/hardware/nano-rp2040-connect/) with me, and since it was a microcontroller with WiFi support, I figured it would be a nice constrained environment for this project.

The most pressing concern was the amount of RAM available. The Nano comes with 264 KB (yes, kilobytes) of SRAM. If we take a traditional RGB colour image with 1 byte per channel, you could only store a maximum of `(264 * 1024) / 3` or ~90,000 pixels, which would come out to a roughly 300x300 px image.

To get larger images, we can exploit a characteristic of our problem: the device that we're viewing from (the Kindle) is grayscale-only. So, all we really need is a single byte to represent the gray level. That gives us ~260,000 pixels or a ~500x500 px image.

We can go even further. The Kindle only supports 16 grayscale levels, which fits exactly in 4 bits, meaning you can fit 2 pixels in one byte. That gives us ~540,000 pixels or a ~700x700 px image.

I opted to go for a 600x700 px image, since we still needed RAM for the rest of the program. That meant the image took up **93%** of available RAM on the microcontroller. This constraint resulted in rather interesting choices for the rest of the program.

### Computing pixel colours on the Arduino
The program is designed to run a particular "draw kernel" over every pixel in the image, essentially a function which takes in the coordinates and returns the corresponding pixel's colour. I'd spotted that the RP2040 is a dual-core microcontroller, meaning I could in theory dedicate an entire core towards drawing the image, while leaving the other core to handle network requests.

Getting the second core to work was non-trivial. [Arduino's official MbedOS base](https://blog.arduino.cc/2021/04/27/arduino-mbed-core-for-rp2040-boards/) was designed for single-core use, but they also include the official [Pico SDK](https://www.raspberrypi.com/documentation/pico-sdk/index_doxygen.html) for lower-level use. The only difficulty being, no Arduino APIs could be called from the second core, else it would crash the microcontroller.

There's an alternate base available called [arduino-pico](https://arduino-pico.readthedocs.io/en/latest/) which better handles the second core (among other things). However, on my board it struggled with the [WiFiNINA library](https://github.com/arduino-libraries/WiFiNINA) needed for interacting with the WiFi module, so I had to scrap that approach.

Since my workload was purely compute-and-memory-only, I went ahead with the official Arduino base instead.

### Drawing pixels randomly
_This was a feature that I didn't anticipate to be challenging!_

My initial implementation drew pixels in a straightforward row-by-row, column-by-column way. However, since it was expected that the pixel colour computation would take a long time to cover the entire image, this meant most of the image wouldn't be visible when viewed.

Instead, I opted to draw pixels in a random order. That way, the image is "dusted in" all over the frame and you can better visualise the partial image. To save on compute time, I also wanted to make sure I drew pixels exactly once per frame.

Now, the straightforward approach to doing this would be to store an array of coordinates and shuffle it to get a permutation. However, we don't have enough RAM to keep all 420,000 pixel indices in memory (You need a 32-bit integer for each number from 0..419,999 ~ 1 MB of RAM just to store the permutation). We need a smarter way that doesn't require extra memory, or in more formal words: we need to generate a permutation of a range of N consecutive integers in O(1) space.

Enter, [this StackOverflow answer](https://stackoverflow.com/a/10054982) and a detour into [Number Theory](https://en.wikipedia.org/wiki/Number_theory) and [Modular Arithmetic](https://en.wikipedia.org/wiki/Modular_arithmetic) land. Specifically, if you have a prime number $P > N$ and a primitive root $g$ ($1 < g < P$, and $g ^ k \mod P = 1$ only when $k$ is a multiple of $(P - 1)$), then $G^k \mod P$ for $k = 1 .. (P - 1)$ will be a permutation of $1..(P-1)$. Incidentally, this is also the underlying concept behind [the RSA encryption algorithm](https://en.wikipedia.org/wiki/RSA_cryptosystem#Operation)!

Since I needed a prime number and a primitive root, I wrote a [quick Python script](./primitive_root_finder.py) to find them. As it turns out, `600 * 700 + 1` was prime, meaning we lucked out on our framebuffer dimensions! However, I accidentally made the script have a cubic time complexity which was unacceptably slow (event for a throwaway, one-time script). A quick check showed it was due to my naive use of the exponentiation operator `a ** b`, and more details can be seen in the script comments.

I want to highlight this particular challenge a bit more: _Even a feature that looked to be cosmetic proved to be quite interesting from a CompSci and Math perspective!_

### Sending images from the Arduino
When it came time to send our image over the network, I had to decide what format to send it in. Initially I attempted to use [PNGenc](https://github.com/bitbank2/PNGenc) to compress our framebuffer, but there wasn't enough free RAM available to store the compressed image buffer. So, we had to send it uncompressed.

Thankfully, [`eips`](https://wiki.mobileread.com/wiki/Eips) (the inbuilt command to draw things to the Kindle's e-ink display) and subsequently [`fbink`](https://github.com/NiLuJe/FBInk) had support for Bitmap files, and Bitmaps can contain raw, uncompressed pixel data. What's more, it even supports the 4 bytes-per-pixel format our framebuffer was using, meaning it was just a matter of copying the framebuffer as-is to the network after sending the BMP header.
