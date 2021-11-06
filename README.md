# Video Blender

Command line video processing tool, weighted frame blending 

### Doc

#### format options

file options :<br/>
`-i file(s)` input(s) file(s)<br/>
`-o folder` output folder

output framerate options :<br/>
`-fps int` set framerate with integer<br/>
`-timebase num/den` set framerate with integer timebase ex:-timebase 1/60

#### codec options

`-dec codec` set decoder<br/>
`-enc codec` set encoder<br/>
`-enc_opt name=value` set encoder options, ex: see default parameters

#### blending options

weights options :<br/>
`-blendweights int` set the number of frames blended with integer weights 1 ex:-blendweights 2<br/>
`-blendweights ints` set integer weights ex:-blendweights 1 1<br/>
`-blendweights floats` set floating weights ex:-blendweights 1.0 1.0

processing options :<br/>
`-blendthreads int` set maximum number of threads for blending, 0 is auto<br/>
`-interbits int` set internal data bits, 32 or 64

#### default parameters

These arguments are the equivalent of default parameters:

`-o ./output -enc libx264 -enc_opt preset=ultrafast crf=0 -fps 60 -blendweights 1 1 -blendthreads 0 -interbits 32`

### Performances infos

Internal data can be handled by 32/64 int/float variables. It will affect processing time and memory usage. You can choose the number of bits with "interbits" parameter. The floating variables are used when you specify floating in "blendweights" argument, integer is default.

If you see blend threads not maxed out, it is a decoder/encoder bottleneck, consider using hardware decoder/encoder.

### planned

cpu optimisations/gpu compute library implementation