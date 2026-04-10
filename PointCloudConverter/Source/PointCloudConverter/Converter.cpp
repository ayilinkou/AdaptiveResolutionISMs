#include <fstream>
#include <filesystem>
#include <array>
#include <random>

#include "Converter.h"
#include "Core/Renderer/Material.h"
#include "Core/Model/Mesh.h"

constexpr UINT RAND_SEED = 12345u;
std::mt19937 rng(RAND_SEED);
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

std::unique_ptr<std::vector<DirectX::XMFLOAT3>> PointCloudConverter::LoadFromFile(std::string_view modelPath, const float density)
{
	std::filesystem::path path(modelPath);
	path.replace_extension(".pc");

	if (!std::filesystem::exists(path))
		return nullptr;

	std::ifstream file(path, std::ios::binary);
	assert(file.is_open());

	float densityInFile;
	size_t pointCount;

	file.read(reinterpret_cast<char*>(&densityInFile), sizeof(float));
	file.read(reinterpret_cast<char*>(&pointCount), sizeof(size_t));

	if (densityInFile != density || pointCount == 0)
		return nullptr;

	auto points = std::make_unique<std::vector<DirectX::XMFLOAT3>>(pointCount);
	file.read(reinterpret_cast<char*>(points->data()), sizeof(DirectX::XMFLOAT3) * pointCount);

	return points;
}

std::unique_ptr<std::vector<DirectX::XMFLOAT3>> PointCloudConverter::LoadFromBuffers(const std::vector<UINT>& indices, const std::vector<Vertex>& vertices,
	const std::unordered_map<Core::Mesh*, std::vector<DirectX::XMMATRIX>> meshLocalTransformsT, const float density)
{
	auto points = std::make_unique<std::vector<DirectX::XMFLOAT3>>();
	for (const auto& [pMesh, transformsT] : meshLocalTransformsT)
	{
		if (!pMesh->GetMaterial()->IsOpaque())
			continue;

		std::vector<DirectX::XMMATRIX> transformsTT(transformsT.size());
		std::transform(
			transformsT.begin(),
			transformsT.end(),
			transformsTT.begin(),
			[](const DirectX::XMMATRIX& m) { return DirectX::XMMatrixTranspose(m); }
		);

		UINT indexOffset = pMesh->GetIndexOffset();
		for (UINT i = 0u; i < pMesh->GetIndexCount(); i += 3)
		{
			const UINT index1 = indices[indexOffset + i + 0u];
			const UINT index2 = indices[indexOffset + i + 1u];
			const UINT index3 = indices[indexOffset + i + 2u];
			const DirectX::XMFLOAT3 v1 = vertices[index1].Pos;
			const DirectX::XMFLOAT3 v2 = vertices[index2].Pos;
			const DirectX::XMFLOAT3 v3 = vertices[index3].Pos;
			const std::array<DirectX::XMFLOAT3, 3> face({ v1, v2, v3 });

			for (const auto& transform : transformsTT)
			{
				AddPointCloudFace(points.get(), face, transformsTT, density);
			}
		}
	}

	return points;
}

void PointCloudConverter::SaveToFile(std::string_view modelPath, const std::vector<DirectX::XMFLOAT3>& points, const float density)
{
	std::filesystem::path path(modelPath);
	path.replace_extension(".pc");

	std::ofstream file(path, std::ios::binary);
	assert(file.is_open());

	size_t pointsCount = points.size();
	file.write(reinterpret_cast<const char*>(&density), sizeof(float));
	file.write(reinterpret_cast<const char*>(&pointsCount), sizeof(size_t));

	if (!points.empty())
		file.write(reinterpret_cast<const char*>(points.data()), sizeof(DirectX::XMFLOAT3) * pointsCount);

	assert(file.good());
}

float PointCloudConverter::TriangleArea(const std::array<DirectX::XMFLOAT3, 3>& face)
{
	const DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&face[0]);
	const DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&face[1]);
	const DirectX::XMVECTOR v3 = DirectX::XMLoadFloat3(&face[2]);

	const DirectX::XMVECTOR edge1 = DirectX::XMVectorSubtract(v2, v1);
	const DirectX::XMVECTOR edge2 = DirectX::XMVectorSubtract(v3, v1);

	const DirectX::XMVECTOR cross = DirectX::XMVector3Cross(edge1, edge2);

	float area = 0.5f * DirectX::XMVectorGetX(DirectX::XMVector3Length(cross));
	return area;
}

DirectX::XMFLOAT3 PointCloudConverter::SamplePointOnTriangle(const std::array<DirectX::XMFLOAT3, 3>& face)
{
	float r1 = std::sqrt(dist(rng));
	float r2 = dist(rng);

	float u = 1.0f - r1;
	float v = r1 * (1.0f - r2);
	float w = r1 * r2;

	const DirectX::XMVECTOR v0 = XMLoadFloat3(&face[0]);
	const DirectX::XMVECTOR v1 = XMLoadFloat3(&face[1]);
	const DirectX::XMVECTOR v2 = XMLoadFloat3(&face[2]);

	const DirectX::XMVECTOR v0u = DirectX::XMVectorScale(v0, u);
	const DirectX::XMVECTOR v1v = DirectX::XMVectorScale(v1, v);
	const DirectX::XMVECTOR v2w = DirectX::XMVectorScale(v2, w);
	const DirectX::XMVECTOR sum = DirectX::XMVectorAdd(DirectX::XMVectorAdd(v0u, v1v), v2w);

	DirectX::XMFLOAT3 out;
	DirectX::XMStoreFloat3(&out, sum);
	return out;
}

void PointCloudConverter::AddPointCloudFace(std::vector<DirectX::XMFLOAT3>* points, const std::array<DirectX::XMFLOAT3, 3>& face,
	const std::vector<DirectX::XMMATRIX>& localTransforms, const float density)
{
	std::vector<DirectX::XMFLOAT3> pointsFromFace = FaceToPointCloud(face, density);
	for (DirectX::XMFLOAT3& p : pointsFromFace)
	{
		for (const DirectX::XMMATRIX& localTransform : localTransforms)
		{
			DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(&p);
			vec = DirectX::XMVector3TransformCoord(vec, localTransform);
			DirectX::XMFLOAT3 transformedPoint;
			DirectX::XMStoreFloat3(&transformedPoint, vec);
			points->push_back(transformedPoint);
		}
	}
}

std::vector<DirectX::XMFLOAT3> PointCloudConverter::FaceToPointCloud(const std::array<DirectX::XMFLOAT3, 3>& face, float density)
{
	std::vector<DirectX::XMFLOAT3> data;

	float area = TriangleArea(face);
	float expected = area * density;
	UINT samples = (UINT)expected; // larger triangles get more samples
	float fractional = expected - (float)samples;

	// Adding a point for each face can cause a high density in areas where there is a large number of small triangles.
	// We still want some small triangles to sometimes contribute to the point cloud.
	// We check the fractional value to stochastically include the contribution of some small triangles in respect to the density.
	float rand = dist(rng);
	if (rand < fractional)
		samples++;

	for (UINT i = 0; i < samples; i++)
	{
		data.push_back(SamplePointOnTriangle(face));
	}

	return data;
}
