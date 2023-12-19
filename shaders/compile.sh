#!/bin/sh

#  compile.sh
#  VulkantTesting
#
#  Created by Hamza Lahmimsi on 2021-02-13.
#  

glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv
glslc grid.vert -o gridVert.spv
glslc grid.frag -o gridFrag.spv
glslc shader.comp -o comp.spv
glslc NoLightingShader.vert -o NoLightingShaderVert.spv
glslc NoLightingShader.frag -o NoLightingShaderFrag.spv
