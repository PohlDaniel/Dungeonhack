#include "DungeonHack.h"
#include "HDR.h"

//---------------------------------------------------------------------------

    /*************************************************************************
    HDRListener Methods
    *************************************************************************/
    //---------------------------------------------------------------------------
    HDRListener::HDRListener()
    {
    }
    //---------------------------------------------------------------------------
    HDRListener::~HDRListener()
    {
    }
    //---------------------------------------------------------------------------
    void HDRListener::notifyViewportSize(int width, int height)
    {
        mVpWidth = width;
        mVpHeight = height;
    }
    //---------------------------------------------------------------------------
    void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
    {
        // Get some RTT dimensions for later calculations
        Ogre::CompositionTechnique::TextureDefinitionIterator defIter = 
            instance->getTechnique()->getTextureDefinitionIterator();
        while (defIter.hasMoreElements())
        {
            Ogre::CompositionTechnique::TextureDefinition* def = 
                defIter.getNext();
            // store the sizes of downscaled textures (size can be tweaked in script)
            if (Ogre::StringUtil::startsWith(def->name, "rt_lum", false))
            {
                int idx = Ogre::StringConverter::parseInt(def->name.substr(6,1));
                mLumSize[idx] = def->width; // should be square
            }
            else if(def->name == "rt_bloom0")
            {
                mBloomSize = def->width; // should be square
                // Calculate gaussian texture offsets & weights
                float deviation = 3.0f;
                float texelSize = 1.0f / (float)mBloomSize;

                // central sample, no offset
                mBloomTexOffsetsHorz[0][0] = 0.0f;
                mBloomTexOffsetsHorz[0][1] = 0.0f;
                mBloomTexWeights[0][0] = mBloomTexWeights[0][1] = 
                    mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
                mBloomTexWeights[0][3] = 1.0f;

                // 'pre' samples
                for(int i = 1; i < 8; ++i)
                {
                    mBloomTexWeights[i][0] = mBloomTexWeights[i][1] = 
                        mBloomTexWeights[i][2] = 1.25f * Ogre::Math::gaussianDistribution(i, 0, deviation);
                    mBloomTexWeights[i][3] = 1.0f;
                    mBloomTexOffsetsHorz[i][0] = i * texelSize;
                    mBloomTexOffsetsHorz[i][1] = 0.0f;
                    mBloomTexOffsetsVert[i][0] = 0.0f;
                    mBloomTexOffsetsVert[i][1] = i * texelSize;
                }
                // 'post' samples
                for(int i = 8; i < 15; ++i)
                {
                    mBloomTexWeights[i][0] = mBloomTexWeights[i][1] = 
                        mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
                    mBloomTexWeights[i][3] = 1.0f;

                    mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
                    mBloomTexOffsetsHorz[i][1] = 0.0f;
                    mBloomTexOffsetsVert[i][0] = 0.0f;
                    mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
                }

            }
        }
    }
    //---------------------------------------------------------------------------
    void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        // Prepare the fragment params offsets
        switch(pass_id)
        {
        //case 994: // rt_lum4
        case 993: // rt_lum3
        case 992: // rt_lum2
        case 991: // rt_lum1
        case 990: // rt_lum0
            {
                // Need to set the texel size
                // Set from source, which is the one higher in the chain
                mat->load();
                Ogre::uint32 idx = pass_id - 990 + 1;
                float texelSize = 1.0f / (float)mLumSize[idx];
                Ogre::GpuProgramParametersSharedPtr fparams = 
                    mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
                fparams->setNamedConstant("texelSize", texelSize);
                break;
            }
        case 800: // rt_brightpass
            break;
        case 701: // rt_bloom1
            {
                // horizontal bloom
                mat->load();
                Ogre::GpuProgramParametersSharedPtr fparams = 
                    mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
                const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
                // A bit hacky - Cg & HLSL index arrays via [0], GLSL does not
                if (progName.find("GLSL") != Ogre::String::npos)
                {
                    fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
                    fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
                }
                else
                {
                    fparams->setNamedConstant("sampleOffsets[0]", mBloomTexOffsetsHorz[0], 15);
                    fparams->setNamedConstant("sampleWeights[0]", mBloomTexWeights[0], 15);
                }

                break;
            }
        case 700: // rt_bloom0
            {
                // vertical bloom 
                mat->load();
                Ogre::GpuProgramParametersSharedPtr fparams = 
                    mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
                const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
                // A bit hacky - Cg & HLSL index arrays via [0], GLSL does not
                if (progName.find("GLSL") != Ogre::String::npos)
                {
                    fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
                    fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
                }
                else
                {
                    fparams->setNamedConstant("sampleOffsets[0]", mBloomTexOffsetsVert[0], 15);
                    fparams->setNamedConstant("sampleWeights[0]", mBloomTexWeights[0], 15);
                }

                break;
            }
        }
    }
    //---------------------------------------------------------------------------
    void HDRListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
    }
    //---------------------------------------------------------------------------
