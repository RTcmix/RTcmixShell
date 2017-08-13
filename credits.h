#ifndef CREDITS_H
#define CREDITS_H

// HTML text for the About box
// See "http://doc.qt.io/qt-5/richtext-html-subset.html" for what is possible.
// Note that table formatting is limited and does not always work as documented.

#define CREDITS \
    "<head><style>" \
    "h4 { margin-top: 20px; }" \
    "p { font-weight: normal; }" \
    "p.tight { font-weight: normal; margin-top: 0px; margin-bottom: 6px; }" \
    "table { margin-top: 10px; }" \
    "td { font-weight: normal; }" \
    "</style></head>" \
    \
    "<p>John Gibson, with Brad Garton and Doug Scott</p>" \
    "<p>using...</p>" \
    \
    "<h4>RTcmix</h4>" \
    "<p>Brad Garton, John Gibson, Doug Scott, David Topper</p>" \
    "<p>with</p>" \
    \
    "<table border=0 width=350>" \
    "<tr>" \
    "  <td>Ivica Bukvic</td>" \
    "  <td>Joel Matthys</td>" \
    "</tr>" \
    "<tr>" \
    "  <td>R. Luke DuBois</td>" \
    "  <td>John Rhoads</td>" \
    "</tr>" \
    "<tr>" \
    "  <td>Mara Helmuth</td>" \
    "  <td>Neil Thornock</td>" \
    "</tr>" \
    "</table>" \
    \
    "<p>based on <b>Cmix</b>, by Paul Lansky, with</p>" \
    \
    "<table border=0 cellpadding=0 cellspacing=0 width=350>" \
    "<tr>" \
    "  <td>Curtis Bahn</td>" \
    "  <td>Romain Kang</td>" \
    "  <td>Andrew Milburn</td>" \
    "</tr>" \
    "<tr>" \
    "  <td>Brad Garton</td>" \
    "  <td>Eric Lyon</td>" \
    "  <td>Doug Scott</td>" \
    "</tr>" \
    "<tr>" \
    "  <td>Lars Graf</td>" \
    "  <td>Dave Madole</td>" \
    "  <td>Charles Sullivan</td>" \
    "</tr>" \
    "<tr>" \
    "  <td>Mara Helmuth</td>" \
    "  <td>&nbsp;</td>" \
    "  <td>&nbsp;</td>" \
    "</tr>" \
    "</table>" \
    \
    "<h4>Thanks</h4>" \
    \
    "<p class=tight>to the <a href=\"http://portaudio.com\">PortAudio</a> project for their audio library</p>" \
    \
    "<p class=tight>to Erik de Castro Lopo for his <a href=\"http://www.mega-nerd.com/libsndfile\">libsndfile</a> sound file library</p>" \
    \
    "<p class=tight>to Perry Cook and Gary Scavone for their <a href=\"http://ccrma.stanford.edu/software/stk\">STK</a> library</p>" \
    \
    "<p class=tight>to Bill Schottstaedt for his " \
    "<a href=\"http://ccrma.stanford.edu/software/snd/sndlib\">sndlib</a> sound file library</a></p>" \
    \
    "<p class=tight>to <a href=\"http://ldesoras.free.fr\">Laurent de Soras</a> for his FFTReal class" \
    \
    "<p class=tight>to Juha Nieminen and Joel Yliluoma for their <a href=\"http://warp.povusers.org/FunctionParser\">FunctionParser</a> library" \
    \
    "<p class=tight>&nbsp;</p>"


/* potentially useful elements...
    in style sheet: "p.spacer { margin-top: 0px; margin-bottom: 0; padding: 0; }" \
    "<p class=spacer>&nbsp;</p>" \
    "<tr><th width=20%></th><th></th></tr>" \
*/

#endif // CREDITS_H
