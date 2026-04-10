#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "DirectXMath.h"

#include "Core/Renderer/Vertex.h"

typedef unsigned int UINT;

namespace Core {
	class Mesh;
}

/*
	Library to process model faces and convert them into a point cloud representation.
	Can save and load from a file to save on processing time at runtime.

	File extension = .pc

	Formatted in binary: 1 float to declare density, 1 size_t for point count, followed by points defined by xyz floats.
*/
class PointCloudConverter
{
public:
	static std::unique_ptr<std::vector<DirectX::XMFLOAT3>> LoadFromFile(std::string_view modelPath, const float density);
	static std::unique_ptr<std::vector<DirectX::XMFLOAT3>> LoadFromBuffers(const std::vector<UINT>& indices, const std::vector<Vertex>& vertices,
		const std::unordered_map<Core::Mesh*, std::vector<DirectX::XMMATRIX>> meshLocalTransformsT, const float density);

	static void SaveToFile(std::string_view modelPath, const std::vector<DirectX::XMFLOAT3>& points, const float density);

private:
	static float TriangleArea(const std::array<DirectX::XMFLOAT3, 3>& face);
	static DirectX::XMFLOAT3 SamplePointOnTriangle(const std::array<DirectX::XMFLOAT3, 3>& face);
	static void AddPointCloudFace(std::vector<DirectX::XMFLOAT3>* points, const std::array<DirectX::XMFLOAT3, 3>& face,
		const std::vector<DirectX::XMMATRIX>& localTransforms, const float density);
	static std::vector<DirectX::XMFLOAT3> FaceToPointCloud(const std::array<DirectX::XMFLOAT3, 3>& face, float density);
};
