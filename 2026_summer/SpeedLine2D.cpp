#include "SpeedLine2D.h"
#include "DxLib.h"
#include "Game.h"

namespace
{
	constexpr int kGenerateEveryFrame = 3;  // 何フレームおきに生成するか
	constexpr int kLineNumPerGenerate = 2;  // 一度に生成する本数
	constexpr int kLineLife = 60;           // 線の寿命(フレーム数)
	constexpr float kLineMinLength = 100.0f;// 線の最小長さ
	constexpr float kLineMaxLength = 300.0f;// 線の最大長さ
	constexpr float kLineMinSpeed = 20.0f;  // 線が流れる速さ(最小)
	constexpr float kLineMaxSpeed = 40.0f;  // 線が流れる速さ(最大)

	constexpr float kAreaTop = 0.0f;	    // 発生エリア(画面上部)の上端
	constexpr float kAreaBottom = 130.0f;	// 発生エリアの下端(この範囲内にランダムで出す)

	constexpr int kScreenWidth = Game::kScreenWidth;
}

SpeedLine2D::SpeedLine2D()
{
}

SpeedLine2D::~SpeedLine2D()
{
}

void SpeedLine2D::TryGenerate(bool isMoving)
{
	if (!isMoving)
	{
		return;
	}

	m_generateInterval++;
	if (m_generateInterval < kGenerateEveryFrame)
	{
		return;
	}
	m_generateInterval = 0;

	for (int i = 0; i < kLineNumPerGenerate; ++i)
	{
		LineData data;

		// 画面右側(奥)からランダムな位置で発生させる
		data.x = kScreenWidth + (rand() % 200); // 画面外右から出発
		data.y = kAreaTop + (rand() % static_cast<int>(kAreaBottom - kAreaTop));
		data.length = kLineMinLength + (rand() % static_cast<int>(kLineMaxLength - kLineMinLength));
		data.speed = kLineMinSpeed + (rand() % static_cast<int>(kLineMaxSpeed - kLineMinSpeed));

		data.life = kLineLife;
		data.maxLife = kLineLife;

		m_lines.push_back(data);
	}
	for (int i = 0; i < kLineNumPerGenerate; ++i)
	{
		LineData data;

		// 画面右側(奥)からランダムな位置で発生させる
		data.x = kScreenWidth + (rand() % 200); // 画面外右から出発
		data.y = (Game::kScreenHeight -( kAreaBottom - kAreaTop)) + (rand() % static_cast<int>(kAreaBottom - kAreaTop));
		data.length = kLineMinLength + (rand() % static_cast<int>(kLineMaxLength - kLineMinLength));
		data.speed = kLineMinSpeed + (rand() % static_cast<int>(kLineMaxSpeed - kLineMinSpeed));

		data.life = kLineLife;
		data.maxLife = kLineLife;

		m_lines.push_back(data);
	}
}

void SpeedLine2D::Update()
{
	for (auto it = m_lines.begin(); it != m_lines.end(); )
	{
		it->x -= it->speed; // 左方向へ流れる(進行方向と逆に見せる)
		it->life--;

		// 寿命切れ、または画面左に出たら削除
		if (it->life <= 0 || it->x + it->length < 0.0f)
		{
			it = m_lines.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void SpeedLine2D::Draw()
{
	for (const auto& line : m_lines)
	{
		float ratio = static_cast<float>(line.life) / line.maxLife;
		int alpha = static_cast<int>(255 * ratio);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawLine(
			static_cast<int>(line.x), static_cast<int>(line.y),
			static_cast<int>(line.x + line.length), static_cast<int>(line.y),
			GetColor(255, 255, 255)
		);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	}
}
