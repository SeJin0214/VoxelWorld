#pragma once

class MapManager
{
public:
	static MapManager& GetInstance() { return instance; }

	// 반환 타입 조금 더 신경 써야 할 듯
	int IsBlockAt(int x, int y, int z) const;
	void RemoveBlockAt(int x, int y, int z);
	int GetRowCount() const { return sizeof(mGrid) / sizeof(mGrid[0]); }
	int GetColumnCount() const { return sizeof(mGrid[0]) / sizeof(mGrid[0][0]); }
	int GetHightCount() const { return sizeof(mGrid[0][0]) / sizeof(bool); }


private:
	static MapManager instance;
	// boolean 배열로 변경 하고 마우스로 삭제만 해보기
	bool mGrid[16][16][16]; // x z y  -> 마인크래프트에서 채용, 빛 투과 계산에 유리하다고 함
	MapManager();
	~MapManager();
	MapManager(const MapManager& other) = delete;
	MapManager& operator=(const MapManager& rhs) = delete;
};
