---
id: ui2_spectrogram
title: The spectrogram
sidebar_label: Spectrogram
slug: /spectrogram-canvas
---

This page will **NOT** go into detail on how to read a spectrogram by itself, for this I can recommend the following video by Zheanna from TransVoiceLessons on YouTube.

<p>
  <div class="video-box">
  <iframe class="video" src="https://www.youtube.com/embed/z4hio1fIDNk" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen="true"></iframe>
  </div>
</p>

Here is an example screenshot of the spectrogram canvas after speaking into the microphone.

![Spectrogram example](/img/ui2.1.png) <center><em>That's a lot of colours!</em></center>

The canvas consists of a classic spectrogram, on top of which pitch estimates and resonance tracks are overlaid.

## Graph axes {#graph-axes}

***The horizontal axis indicates the time in seconds.***  
Pausing the analysis will cause the time axis to stop progressing.

***The vertical axis indicates frequency in Hertz.***  
The scale is set to ERB by default, it is a non-linear frequency scale that is modelled after human psychoacoustics.

## Frequency tracks {#frequency-tracks}

InFormant displays various frequency tracks as dots of different colours.
*The latest version as of writing this documentation (3.0.2) does not allow customisation of those colours.*

### Pitch, F<sub>o</sub> {#pitch-fsubosub}

**The cyan track represents your estimated pitch.**

![Zoom on pitch track](/img/ui2.2.png)

*These pitch estimation algorithms are only correct within a certain frequency range, usually somewhere between 60 and 600 Hz.*

### R<sub>1</sub>, R<sub>2</sub>, R<sub>3</sub> {#rsub1sub-rsub2sub-rsub3sub}

**The three other tracks represent your estimated resonances.**

![Zoom on R1,R2,R3 track](/img/ui2.3.png)

They are always ordered by frequency, which is not always correct from a semantic point of view.
