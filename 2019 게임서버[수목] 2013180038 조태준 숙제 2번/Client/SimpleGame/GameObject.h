#pragma once

class CGameObject
{
protected:
	float m_fx;
	float m_fy;
	float m_fz;
	float m_fsize;

	// R G B A
	float m_fred;
	float m_fgreen;
	float m_fblue;
	float m_falpha;

	int m_ntype;

	float m_fbulidlevel;

	unsigned int m_ntexID;
public:
	CGameObject();
	~CGameObject();
public:
	virtual void Render();
	virtual void Update(float time);
	virtual void KeyInput(int key) = 0;

	inline float GetX() {
		return m_fx;
	}
	inline float GetY() {
		return m_fy;
	}
	inline float GetZ() {
		return m_fz;
	}
	inline float GetSize() {
		return m_fsize;
	}
	inline float GetRed() {
		return m_fred;
	}
	inline float GetGreen() {
		return m_fgreen;
	}
	inline float GetBlue() {
		return m_fblue;
	}
	inline float GetAlpha() {
		return m_falpha;
	}
	inline unsigned int GettexID() {
		return m_ntexID;
	}
	inline int GetType() {
		return m_ntype;
	}

	inline void SetPosition(float x, float y, float z) {
		m_fx = x; m_fy = y; m_fz = z;
	}
	inline void SetColor(float r, float g, float b, float a) {
		m_fred = r; m_fgreen = g; m_fblue = b; m_falpha = a;
	}
	inline void SettexID(unsigned int texid) {
		m_ntexID = texid;
	}
	inline void SetType(int type) {
		m_ntype = type;
	}
	
public:
	virtual void Release() = 0;
};