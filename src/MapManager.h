#pragma once


class MapManager
{
public:
	static MapManager& GetInstance() { return instance; }
	// 각 그리드의 높이값을 2차원 배열로 반환
	// 반환 타입 조금 더 신경 써야 할 듯
	int GetHeightAt(int x, int z) const;
	int GetRowCount() const { return sizeof(mGridHeights) / sizeof(mGridHeights[0]); }
	int GetColumnCount() const { return sizeof(mGridHeights[0]) / sizeof(int); }


private:
	static MapManager instance;
	// boolean 배열로 변경 하고 마우스로 삭제만 해보기
	int mGridHeights[16][16];
	MapManager();
	~MapManager();
	MapManager(const MapManager& other) = delete;
	MapManager& operator=(const MapManager& rhs) = delete;
};
