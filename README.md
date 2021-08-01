# Video Blender

Command line video processing tool, lossless weighted frame blending 

## How to use

### Exemple

blend 3 frames
 * `vblender.exe -i video.mp4 -weights 3`

### Doc

file options :
`-i file` input file
`-o file` output file

output framerate options :
`-fps int` set timebase with framerate
`-timebase num/den` set timebase ex:-timebase 1/60

weights options :
`-weights int` set the number of frames blended with weights 1
`-weights ints` set weights ex:-weights 1 1

processing options :
`-threads int` set number of threads for blending
`-opengl` use opengl to blend frames

codec options :
`-decoder codec` set decoder, possible values:h264,h265
`-preset preset` set decoder preset
`-crf int` set quality
`-hd dec` set hardware decoder, possible values:Intel,Nvidia
`-he enc` set hardware encoder, possible values:Intel,Nvidia,AMD

## Why

Streaming platforms and people's monitors are often maxed to 60 fps. If you have videos with more than 60 fps, you have two choices :
 * drop the framerate to keep sharpness
 * blend frames to keep the motion

To achieve frame blending you can use editing softwares but you can't configure frame's weights, I used ffmpeg's tmix + fps filters but it is very slow because it calculate a lot of frames who are dropped to achieve targeted framerate.

## Todo

 * clean the code
 * more than 8 bit colors support
 * real number weights
 * remux other input streams
 * port to other os (Windows only actually)

## Links

Some stuff related to frame blending : 

 * [Editing Software Frame Blending Weights Estimation](https://github.com/unknownopponent/Editing-Software-Frame-Blending-Weights-Estimation) : github repo about editing software frame blending
 * [my yt channel](https://www.youtube.com/channel/UCGeOcmuVh36YwHy_fvXrfng/videos) : showcase frame blending
 * [Couleur tweak tips](https://discord.gg/ctt) : video quality/frame blending discord server (and much more)