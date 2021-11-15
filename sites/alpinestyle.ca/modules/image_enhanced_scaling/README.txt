// $Id: README.txt,v 1.4.2.1 2007/01/23 14:02:11 zoo33 Exp $

image_enhanced_scaling.module

Image Enhanced Scaling replaces the image scaling function used by the 
Image module with one that gives a better image quality. This is achieved 
by applying a sharpness filter to the derivative images and converting 
them to a suitable color profile. The module also converts all derivative 
images to JPEG and sets their resolution to 72 ppi.


DEPENDENCIES
------------
This module depends on image.module and requires that the ImageMagick 
framework provided by the image module is installed and properly 
configured. The GD framework won't work. This also means that you have to 
have ImageMagick installed on the server.


INSTALLATION
------------
First, make sure you have image.module installed, activated and working.
Then activate the ImageMagick framework. This is done by moving the file 
image.imagemagick.inc from the image module's directory to Drupal's 
includes directory and then changing the image framwork setting at 
Administer >> Site configuration >> Image toolkit. Verify that it's 
working by adding at least one image before proceeding.

Install this module by copying this directory to your modules directory. 
Enable it at administer >> modules. You may then go to Administer >> 
Site configuration >> Image enhanced scaling and configure the module's 
settings.


TROUBLESHOOTING
---------------

* If the color profile is not applied to your derivative images and you 
are positive that you have specified the correct path to the color profile, 
there is likely a problem with special characters in the path. These are 
the characters that are allowed:

A-Z a-z 0-9 . - + [space] / \ _

(In earlier versions of this module slashes (/) weren't accepted, which 
probably caused the color conversion to fail in most cases.)


KNOWN ISSUES
------------
* There is an overhead with this module since the scaling to each derivative 
size is actually performed twice: first by image.module and then by 
image_enhanced_scaling.module. On a site where images are not added or 
edited too often this should not be a problem. However, you should be aware 
that changing any of the settings on this module's or the Image module's 
settings page will cause all image derivatives to be rebuilt, so try to do 
that as little as possible and preferrably only when setting up the site.

To solve this problem, you can disable the extra scaling that this module 
does at administer >> settings >> image_enhanced_scaling and then modify 
image.module as described below (this has not been tested).

Replace this line (wrapped in this file):

        if (!image_scale($source, file_create_path($destination), 
          $size['width'], $size['height'])) {

with these:

        if (!image_enhanced_scaling_resize(
          $source, file_create_path($destination), 
          $size['width'], $size['height'], 
          variable_get('image_enhanced_scaling_sharp_amount',90), 
          variable_get('image_enhanced_scaling_sharp_radius',0.9), 
          variable_get('image_enhanced_scaling_profile','')
        )) {

* Disabling and enabling the enhanced scaling at administer >> settings >>
image_enhanced_scaling will unnecessarily force all image derivatives to 
be rebuilt.

* This module has a minor issue when used in combination with 
image_exact.module. The two modules use the same technique for bypassing 
image.module's scaling, but only the one that is loaded last will take 
effect. In this case image_exact will handle the thumbnail generation since 
it comes last alphabetically. In order to have both modules' functionality 
applied to thumbnails, you'll probably have to modify image_exact.module as 
described below (not tested).

Replace the following line:

    image_resize($destination, $destination, $final_w, $final_h);

with these:

    image_enhanced_scaling_resize(
      $destination, $destination, $final_w, $final_h, 
      variable_get('image_enhanced_scaling_sharp_amount',90), 
      variable_get('image_enhanced_scaling_sharp_radius',0.9), 
      variable_get('image_enhanced_scaling_profile','')
    );


CREDITS
-------
Based on image_exact.module and parts of image.module.
Developed by Hannes Lilljequist (zoo33 at drupal.org) with support by 
ETC bild (http://www.etcbild.se).


LICENSE
-------
This module is released under GPL. See LICENSE.txt
