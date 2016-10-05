#pragma once

#include "mLibInclude.h"

#include <cuda_runtime.h>
#include <cudaUtil.h>

#include "CUDAWarpingSolver.h"
#include "TerraSolver.h"

class ImageWarping
{
	public:
	
		ImageWarping(const ColorImageR32G32B32A32& image)
		{
			m_image = image;

			cutilSafeCall(cudaMalloc(&d_imageFloat3,  sizeof(float4)*m_image.getWidth()*m_image.getHeight()));
			cutilSafeCall(cudaMalloc(&d_targetFloat3, sizeof(float4)*m_image.getWidth()*m_image.getHeight()));

			resetGPUMemory();

			m_warpingSolver = new CUDAWarpingSolver(m_image.getWidth(), m_image.getHeight());
            m_terraSolver = new TerraSolver(m_image.getWidth(), m_image.getHeight(), "SmoothingLaplacianFloat3AD.t", "gaussNewtonGPU");
		}

		void resetGPUMemory()
		{
			float3* h_imageFloat3 = new float3[m_image.getWidth()*m_image.getHeight()];

			for (unsigned int i = 0; i < m_image.getHeight(); i++)
			{
				for (unsigned int j = 0; j < m_image.getWidth(); j++)
				{
					ml::vec4f v = m_image(j, i);
					h_imageFloat3[i*m_image.getWidth() + j] = make_float3(v.x, v.y, v.z);
				}
			}

			cutilSafeCall(cudaMemcpy(d_imageFloat3, h_imageFloat3, sizeof(float3)*m_image.getWidth()*m_image.getHeight(), cudaMemcpyHostToDevice));
			cutilSafeCall(cudaMemcpy(d_targetFloat3, h_imageFloat3, sizeof(float3)*m_image.getWidth()*m_image.getHeight(), cudaMemcpyHostToDevice));

			delete h_imageFloat3;
		}

		~ImageWarping()
		{
			cutilSafeCall(cudaFree(d_imageFloat3));
			cutilSafeCall(cudaFree(d_targetFloat3));

			SAFE_DELETE(m_warpingSolver);
		}

		ColorImageR32G32B32A32* solve()
		{
			float weightFit  = 1.0f;
			float weightReg  = 800.0f;
			float pNorm = 1.0f;

			unsigned int nonLinearIter = 50;
			unsigned int linearIter = 40;
		
			std::cout << "\n\nCUDA" << std::endl;
			resetGPUMemory();
			m_warpingSolver->solveGN(d_imageFloat3, d_targetFloat3, nonLinearIter, linearIter, weightFit, weightReg, pNorm);
			copyResultToCPUFromFloat3();

            std::cout << "\n\nOPT" << std::endl;
            resetGPUMemory();
            m_terraSolver->solve(d_imageFloat3, d_targetFloat3, nonLinearIter, linearIter, 0, weightFit, weightReg, pNorm);
            copyResultToCPUFromFloat3();
			
			return &m_result;
		}

		void copyResultToCPUFromFloat3()
		{
			m_result = ColorImageR32G32B32A32(m_image.getWidth(), m_image.getHeight());

			float3* h_imageFloat3 = new float3[m_image.getWidth()*m_image.getHeight()];
			cutilSafeCall(cudaMemcpy(h_imageFloat3, d_imageFloat3, sizeof(float3)*m_image.getWidth()*m_image.getHeight(), cudaMemcpyDeviceToHost));

			for (unsigned int i = 0; i < m_image.getHeight(); i++)
			{
				for (unsigned int j = 0; j < m_image.getWidth(); j++)
				{
					float3 v = h_imageFloat3[i*m_image.getWidth() + j];
					m_result(j, i) = vec4f(v.x, v.y, v.z, 1.0f);
				}
			}

			delete h_imageFloat3;
		}

	private:

		ColorImageR32G32B32A32 m_result;
		ColorImageR32G32B32A32 m_image;
	
		float3*	d_imageFloat3;
		float3* d_targetFloat3;
	
		CUDAWarpingSolver*		m_warpingSolver;
        TerraSolver* m_terraSolver;
        
};
