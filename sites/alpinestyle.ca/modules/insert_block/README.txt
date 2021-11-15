OVERVIEW
--------
Sidebar blocks contain all sorts of nifty stuff, but sometimes you want to stick that stuff into the body of your node. Instead of using PHP snippets (a possible security hole on public sites), you can use this module. When it's activated...

[block:<name of module>=<delta of block>]

...will insert the contents of a rendered sidebar block into the body of your node. If no delta is specified, the default block for that module will be displayed.


INSTALLATION
------------
Drop it into your modules folder and turn it on. Woo!