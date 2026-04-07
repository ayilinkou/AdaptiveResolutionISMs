#pragma once

#include "d3d11.h"

template <typename T>
struct ShaderCreateInfo;

template<>
struct ShaderCreateInfo<ID3D11VertexShader>
{
    static constexpr const char* Target = "vs_5_0";
    static constexpr const char* Suffix = " vertex shader";

    static HRESULT Create(ID3D11Device* device, ID3D10Blob* blob, ID3D11VertexShader** outShader)
    {
        return device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11HullShader>
{
    static constexpr const char* Target = "hs_5_0";
    static constexpr const char* Suffix = " hull shader";

    static HRESULT Create(ID3D11Device* device, ID3D10Blob* blob, ID3D11HullShader** outShader)
    {
        return device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11DomainShader>
{
    static constexpr const char* Target = "ds_5_0";
    static constexpr const char* Suffix = " domain shader";

    static HRESULT Create(ID3D11Device* device, ID3D10Blob* blob, ID3D11DomainShader** outShader)
    {
        return device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11GeometryShader>
{
    static constexpr const char* Target = "gs_5_0";
    static constexpr const char* Suffix = " geometry shader";

    static HRESULT Create(ID3D11Device* device, ID3D10Blob* blob, ID3D11GeometryShader** outShader)
    {
        return device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11PixelShader>
{
    static constexpr const char* Target = "ps_5_0";
    static constexpr const char* Suffix = " pixel shader";

    static HRESULT Create(ID3D11Device* device, ID3D10Blob* blob, ID3D11PixelShader** outShader)
    {
        return device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outShader);
    }
};

template<>
struct ShaderCreateInfo<ID3D11ComputeShader>
{
    static constexpr const char* Target = "cs_5_0";
    static constexpr const char* Suffix = " compute shader";

    static HRESULT Create(ID3D11Device* device, ID3D10Blob* blob, ID3D11ComputeShader** outShader)
    {
        return device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, outShader);
    }
};
