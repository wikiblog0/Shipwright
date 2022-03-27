#ifdef __WIIU__

#include "gx2_shader_debug.h"
#include <stdio.h>
#include <string.h>

#include <gx2/temp.h>

int dumpGfdFile(uint64_t shader_id0, uint32_t shader_id1, struct ShaderGroup *group)
{
    char filename[512];
    sprintf(filename, "shaders/%016llx-%08x.gfd", shader_id0, shader_id1);

    FILE* f = fopen(filename, "wb");
    if (!f) {
        return -1;
    }

    // write the gfd header
    GFDHeader header = {};
    header.magic = GFD_HEADER_MAGIC;
    header.headerSize = sizeof(GFDHeader);
    header.majorVersion = GFD_FILE_VERSION_MAJOR;
    header.minorVersion = GFD_FILE_VERSION_MINOR;
    header.gpuVersion = GX2TempGetGPUVersion();
    fwrite(&header, 1, sizeof(header), f);

    {
        // write the vertex shader block header
        GFDBlockHeader vertexHeader = {};
        vertexHeader.magic = GFD_BLOCK_HEADER_MAGIC;
        vertexHeader.headerSize = sizeof(GFDBlockHeader);
        vertexHeader.majorVersion = GFD_BLOCK_VERSION_MAJOR;
        vertexHeader.minorVersion = 0;
        vertexHeader.type = GFD_BLOCK_VERTEX_SHADER_HEADER;
        vertexHeader.dataSize = sizeof(GX2VertexShader) + sizeof(GFDRelocationHeader);
        vertexHeader.id = 0;
        vertexHeader.index = 0;
        fwrite(&vertexHeader, 1, sizeof(vertexHeader), f);

        GX2VertexShader tmpVsh;
        memcpy(&tmpVsh, &group->vertexShader, sizeof(tmpVsh));

        // program offset needs to be NULL
        tmpVsh.program = NULL;

        // we would technically need to write those fields to the block too and do relocation entries
        // but I'm lazy so just skip that
        tmpVsh.uniformBlocks = NULL;
        tmpVsh.uniformBlockCount = 0;

        tmpVsh.uniformVars = NULL;
        tmpVsh.uniformVarCount = 0;

        tmpVsh.initialValues = NULL;
        tmpVsh.initialValueCount = 0;

        tmpVsh.loopVars = NULL;
        tmpVsh.loopVarCount = 0;

        tmpVsh.samplerVars = NULL;
        tmpVsh.samplerVarCount = 0;

        tmpVsh.attribVars = NULL;
        tmpVsh.attribVarCount = 0;

        fwrite(&tmpVsh, 1, sizeof(tmpVsh), f);

        // write an empty relocation header
        GFDRelocationHeader vertexRelHeader = {};
        vertexRelHeader.magic = GFD_RELOCATION_HEADER_MAGIC;
        vertexRelHeader.headerSize = sizeof(GFDRelocationHeader);
        fwrite(&vertexRelHeader, 1, sizeof(vertexRelHeader), f);

        // write the vertex shader program block header
        GFDBlockHeader vertexProgramHeader = {};
        vertexProgramHeader.magic = GFD_BLOCK_HEADER_MAGIC;
        vertexProgramHeader.headerSize = sizeof(GFDBlockHeader);
        vertexProgramHeader.majorVersion = GFD_BLOCK_VERSION_MAJOR;
        vertexProgramHeader.minorVersion = 0;
        vertexProgramHeader.type = GFD_BLOCK_VERTEX_SHADER_PROGRAM;
        vertexProgramHeader.dataSize = group->vertexShader.size;
        vertexProgramHeader.id = 0;
        vertexProgramHeader.index = 0;
        fwrite(&vertexProgramHeader, 1, sizeof(vertexProgramHeader), f);
        
        // write the vertex shader program data
        fwrite(group->vertexShader.program, 1, group->vertexShader.size, f);
    }

    {
        // write the pixel shader block header
        GFDBlockHeader pixelHeader = {};
        pixelHeader.magic = GFD_BLOCK_HEADER_MAGIC;
        pixelHeader.headerSize = sizeof(GFDBlockHeader);
        pixelHeader.majorVersion = GFD_BLOCK_VERSION_MAJOR;
        pixelHeader.minorVersion = 0;
        pixelHeader.type = GFD_BLOCK_PIXEL_SHADER_HEADER;
        pixelHeader.dataSize = sizeof(GX2PixelShader) + sizeof(GFDRelocationHeader);
        pixelHeader.id = 0;
        pixelHeader.index = 0;
        fwrite(&pixelHeader, 1, sizeof(pixelHeader), f);

        GX2PixelShader tmpPsh;
        memcpy(&tmpPsh, &group->pixelShader, sizeof(tmpPsh));

        uint32_t offset = sizeof(GX2PixelShader);

        // program offset always needs to be NULL
        tmpPsh.program = NULL;

        // we would technically need to write those fields to the block too and do relocation entries
        // but I'm lazy so just skip that
        tmpPsh.uniformBlocks = NULL;
        tmpPsh.uniformBlockCount = 0;

        tmpPsh.uniformVars = NULL;
        tmpPsh.uniformVarCount = 0;

        tmpPsh.initialValues = NULL;
        tmpPsh.initialValueCount = 0;

        tmpPsh.loopVars = NULL;
        tmpPsh.loopVarCount = 0;

        tmpPsh.samplerVars = NULL;
        tmpPsh.samplerVarCount = 0;

        fwrite(&tmpPsh, 1, sizeof(tmpPsh), f);

        // write an empty relocation header
        GFDRelocationHeader pixelRelHeader = {};
        pixelRelHeader.magic = GFD_RELOCATION_HEADER_MAGIC;
        pixelRelHeader.headerSize = sizeof(GFDRelocationHeader);
        fwrite(&pixelRelHeader, 1, sizeof(pixelRelHeader), f);

        // write the pixel shader program block header
        GFDBlockHeader pixelProgramHeader = {};
        pixelProgramHeader.magic = GFD_BLOCK_HEADER_MAGIC;
        pixelProgramHeader.headerSize = sizeof(GFDBlockHeader);
        pixelProgramHeader.majorVersion = GFD_BLOCK_VERSION_MAJOR;
        pixelProgramHeader.minorVersion = 0;
        pixelProgramHeader.type = GFD_BLOCK_PIXEL_SHADER_PROGRAM;
        pixelProgramHeader.dataSize = group->pixelShader.size;
        pixelProgramHeader.id = 0;
        pixelProgramHeader.index = 0;
        fwrite(&pixelProgramHeader, 1, sizeof(pixelProgramHeader), f);
        
        // write the pixel shader program data
        fwrite(group->pixelShader.program, 1, group->pixelShader.size, f);
    }

    // write eof header
    GFDBlockHeader eofHeader;
    eofHeader.magic = GFD_BLOCK_HEADER_MAGIC;
    eofHeader.headerSize = sizeof(GFDBlockHeader);
    eofHeader.majorVersion = GFD_BLOCK_VERSION_MAJOR;
    eofHeader.minorVersion = 0;
    eofHeader.type = GFD_BLOCK_END_OF_FILE;
    fwrite(&eofHeader, 1, sizeof(eofHeader), f);

    fclose(f);

    return 0;
}

int dumpShaderFeatures(uint64_t shader_id0, uint32_t shader_id1, struct CCFeatures *cc_features)
{
    char filename[512];
    sprintf(filename, "shaders/%016llx-%08x.txt", shader_id0, shader_id1);

    FILE* f = fopen(filename, "wb");
    if (!f) {
        return -1;
    }

    fprintf(f, "Shader ID 0: %016llx\n", shader_id0);
    fprintf(f, "Shader ID 1: %08x\n\n", shader_id1);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 4; k++) {
                fprintf(f,"c[%d][%d][%d]: %02x\n", i, j, k, cc_features->c[i][j][k]);
            }
        }
    }

    fprintf(f, "\n");

    fprintf(f, "opt_alpha: %d\n", cc_features->opt_alpha);
    fprintf(f, "opt_fog: %d\n", cc_features->opt_fog);
    fprintf(f, "opt_texture_edge: %d\n", cc_features->opt_texture_edge);
    fprintf(f, "opt_noise: %d\n", cc_features->opt_noise);
    fprintf(f, "opt_2cyc: %d\n", cc_features->opt_2cyc);
    fprintf(f, "opt_alpha_threshold: %d\n", cc_features->opt_alpha_threshold);
    fprintf(f, "opt_invisible: %d\n", cc_features->opt_invisible);

    fprintf(f, "\n");

    for (int i = 0; i < 2; i++) {
        fprintf(f, "used_textures[%d]: %d\n", i, cc_features->used_textures[i]);
        fprintf(f, "clamp[%d][0]: %d\n", i, cc_features->clamp[i][0]);
        fprintf(f, "clamp[%d][1]: %d\n", i, cc_features->clamp[i][1]);
    }

    fprintf(f, "\n");

    fprintf(f, "num_inputs: %d\n", cc_features->num_inputs);

    fprintf(f, "\n");

    fprintf(f, "do_single[0][0]: %d\n", cc_features->do_single[0][0]);
    fprintf(f, "do_single[0][1]: %d\n", cc_features->do_single[0][1]);
    fprintf(f, "do_single[1][0]: %d\n", cc_features->do_single[1][0]);
    fprintf(f, "do_single[1][1]: %d\n", cc_features->do_single[1][1]);

    fprintf(f, "\n");

    fprintf(f, "do_multiply[0][0]: %d\n", cc_features->do_multiply[0][0]);
    fprintf(f, "do_multiply[0][1]: %d\n", cc_features->do_multiply[0][1]);
    fprintf(f, "do_multiply[1][0]: %d\n", cc_features->do_multiply[1][0]);
    fprintf(f, "do_multiply[1][1]: %d\n", cc_features->do_multiply[1][1]);

    fprintf(f, "do_mix[0][0]: %d\n", cc_features->do_mix[0][0]);
    fprintf(f, "do_mix[0][1]: %d\n", cc_features->do_mix[0][1]);
    fprintf(f, "do_mix[1][0]: %d\n", cc_features->do_mix[1][0]);
    fprintf(f, "do_mix[1][1]: %d\n", cc_features->do_mix[1][1]);

    fprintf(f, "\n");

    fclose(f);
}

#endif
