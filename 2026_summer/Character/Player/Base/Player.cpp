#include "Player.h"
#include "PlayerState.h"
#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"
#include "../Weapon.h"
#include "../../../DataLoader/DataManager.h"
#include "../../../System.h"
#include "../../../Math/Matrix4x4.h"
#include "../../../Camera/Camera.h"
#include "../../../SubWindow/SubWindow.h"
#include "../../../Input.h"
#include "EffekseerForDXLib.h"
#include <cmath>
#include <cassert>
#include <string>
#include <fstream>
#include <sstream>

namespace
{
	constexpr float kPlayerCenter = 100.0f;//プレイヤーの当たり判定の中心点までのy軸の距離
}


Player::Player()
{
	//m_modelHandle = MV1LoadModel("data/Player/Player.mv1");
	//m_modelHandle = MV1LoadModel("data/Player/Player_Init.mv1");
	m_modelHandle = MV1LoadModel("data/Player/Player_true.mv1");
	//モデルの初期位置を設定する//前を向いているようにする
	Matrix4x4 rotY = Matrix4x4::MakeRotationY(DX_PI_F);
	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);
	Matrix4x4 mtx = trans * rotY;
	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
	
	//コンボチェーンの初期化
	InitializeComboChain();
	//アニメーションの名前のマップの初期化
	const auto& animData = DataManager::GetInstance().GetPlayerAnimData();
	m_animNames = animData.animNames;

	//鴉の羽のモデルの読み込み
	m_wingModelHandle = MV1LoadModel("data/Player/Weapon/Wing/wing.mv1");
	WingUpdate();//鴉状態の羽の更新
}

Player::~Player()
{
	if(m_currentState)
	{
		m_currentState->Exit();//状態を抜ける
	}
	MV1DeleteModel(m_modelHandle);
	MV1DeleteModel(m_wingModelHandle);
	DeleteEffekseerEffect(m_efHandle);
}

void Player::Init()
{
	//向いている方向
	m_targetVec = Vector3(0, 0, 1);//最初は前を向いているようにする
	//初期状態をIdleにする//アニメーションの初期化
	m_anim.Init(m_modelHandle,GetAnimName("Idle"), true);
	//weak_from_this()は、shared_ptrを作成,
	//Playerクラスのインスタンスから、Playerクラスのshared_ptrを取得できるようになる
	m_currentState = std::make_shared<PlayerStateIdle>(weak_from_this());

	//IDの取得
	SetID();
	//当たり判定の初期化
	//中心点、オフセット、半径、当たり判定のタイプ、タグ、当たり判定が有効かどうか
	ColInit(m_pos, Vector3(0, kPlayerCenter, 0), 80.0f, ColliderType::Sphere, Tags::Player, true);
	//やられ判定の初期化
	InitHitCol(weak_from_this());
	m_hitCol->ColInit(m_pos, Vector3(0, kPlayerCenter, 0), 50.0f, ColliderType::Sphere, Tags::PlayerHit, true,true);
	CharacterBase::ApplyPos();//座標の更新//モデルの座標を更新する
	ChangeState(m_currentState);//初期化
	//武器の生成
	m_weapon = std::make_shared<Weapon>(weak_from_this());//武器の生成//Playerクラスのインスタンスから、Playerクラスのshared_ptrを取得できるようになる
	//effectの生成
	m_efHandle = LoadEffekseerEffect("data/Effect/fire.efk",1.0f);
}

void Player::Update(Camera& camera)
{
	//カメラの更新
	m_camera = &camera;
	//ライト(雑実装)
	m_camera->UpdateLight();
	auto& input = Input::GetInstance();

	//回避のクールタイムの更新
	if (m_avoidInfo.avoidCoolTimeCount > 0.0f)
	{
		m_avoidInfo.avoidCoolTimeCount -= 1.0f * System::GetInstance().GetTimeScale();//回避のクールタイムを減らす
	}
	

	//押し戻しの処理が続かないように消す
	m_vel = Vector3(0, m_vel.y, 0);

	if (m_currentState)
	{
		m_currentState->Update();//状態の更新
	}


	//これらは押し戻しの時に呼ばれないのでずれる→そこでも呼ぶ必要あり
	WingUpdate();
	m_weapon->Update();//武器の更新
	// 定期的にエフェクトを再生する
	static int time = 0;
	if (time++ % 60 == 0)
	{
		// エフェクトを再生する。
		m_efPlayingHandle = PlayEffekseer3DEffect(m_efHandle);

		// エフェクトの位置をリセットする。
	}
	

	//座標の更新の前に、当たり判定の更新をする

	//座標の更新
	//float timeScale = System::GetInstance().GetTimeScale();
	//m_pos += m_vel * timeScale;

	//回転処理//座標も行列で更新
	//UpdateAngle();
	//UpdateAngleAndPos();
	 
	//-----------------------変わらなかった；；-----------------------------------------
	////ルートモーションONの場合、
	////アニメーションが適用された後のモデルの行列を取得
	//MATRIX modelMat = MV1GetMatrix(m_modelHandle);

	////モデルの行列から、移動量を取得する
	//m_pos.x = modelMat.m[3][0];
	//m_pos.y = modelMat.m[3][1];
	//m_pos.z = modelMat.m[3][2];
	////Y軸回転角度を合わせる
	//m_rotAngleY = atan2f(modelMat.m[0][2], modelMat.m[2][2]);//モデルの行列から、Y軸の回転角度を取得する

	////方向ベクトルを同期
	//m_targetVec.x = sinf(m_rotAngleY + DX_PI_F);
	//m_targetVec.z = cosf(m_rotAngleY + DX_PI_F);
	//m_targetVec.y = 0.0f;
	//m_targetVec = m_targetVec.Normalize();
	//----------------------------------------------------------------------------------------
}
void Player::Draw()
{
	MV1DrawModel(m_modelHandle);
	//鴉状態のときのみ描画
	if(m_isRaven)MV1DrawModel(m_wingModelHandle);
	m_weapon->Draw();//武器
	//コンボチェーンの描画
	for(int i = 0; i < m_comboChain.size(); ++i)
	{
		std::string text = std::to_string(i) + ": " + m_comboChain[i].animName;
		//SubWindow::AddText(text);
	}
	std::string posText = "Pos: " + std::to_string(m_pos.x) + ", " + std::to_string(m_pos.y) + ", " + std::to_string(m_pos.z);
	SubWindow::AddText(posText);
	std::string velText = "Vel: " + std::to_string(m_vel.x) + ", " + std::to_string(m_vel.y) + ", " + std::to_string(m_vel.z);
	SubWindow::AddText(velText);
	std::string ravenText = "Raven: " + std::string(m_isRaven ? "true" : "false");
	SubWindow::AddText(ravenText);
#ifdef _DEBUG
	if (m_currentState)
	{
		m_currentState->DebugDraw();//デバッグ描画
	}
#endif
	DrawSphere3D(m_pos.ToDxLibVector(), kPlayerRockOnRange, 16, GetColor(255, 0, 0), GetColor(255, 0, 0), false);
}

void Player::EffectDraw()
{
	// 再生中のエフェクトを移動する。
	SetPosPlayingEffekseer3DEffect(m_efPlayingHandle, m_pos.x, m_pos.y, m_pos.z);
	//SetColorPlayingEffekseer3DEffect(m_efPlayingHandle, 255, 255, 255, 255);
}

void Player::OnCollision(Collider& other)
{
}

void Player::OnDamage(Collider& other, AttackData& data)
{
	
	//ダメージを受けたときの処理
	//敵の攻撃データをもらい、ダメージを減らし、体力を減らす、場合によってはプレイヤーを吹き飛ばす
	DrawFormatString(0, 0, GetColor(255, 0, 0), "Player: OnDamage");
	//ダメージを受けたときの処理
	
	//StateがAvoidで回避中かつ、時間内だったらジャスト回避
	auto nowState = std::dynamic_pointer_cast<PlayerStateAvoid>(m_currentState);
	if(m_currentState == nowState)
	{

	}


	//IsHitStateに変える//後でする



}

void Player::ChangeState(std::shared_ptr<PlayerState> newState)
{
	//現在の状態から抜ける
	if(m_currentState)
	{
		m_currentState->Exit();
	}
	//newStateに更新
	m_currentState = newState;
	//newStateの初期化
	if(m_currentState)
	{
		m_currentState->Enter();
	}
}

void Player::InitializeComboChain()
{
	//コンボチェーンの初期化
	//音の出すタイミングは、攻撃の当たり判定を有効にするタイミングと同じにする予定なので、コンボノードの中に入れる

	//CSVファイルを読み込む
	const auto& rawData = DataManager::GetInstance().GetComboRawData();

	for (const auto& tokens : rawData)
	{
		//列数チェック//tokensはvector<string>で、1行分のデータが入っている//横
		if (tokens.size() < ComboNodeType::Size)//vectorのサイズなので、push_backした行の数
		{
			assert(false && "ComboChain.csvの列数が不足しています");
			continue;
		}
		ComboNode node;
		node.animName = tokens[ComboNodeType::AnimName];
		node.type = static_cast<AttackType>(std::stoi(tokens[ComboNodeType::Type]));
		node.index = std::stoi(tokens[ComboNodeType::Index]);
		node.attackPower = std::stof(tokens[ComboNodeType::AttackPower]);
		node.moveFrame = std::stof(tokens[ComboNodeType::MoveTimeRate]);
		node.moveSpeedX = std::stof(tokens[ComboNodeType::MoveSpeedX]);
		node.moveSpeedY = std::stof(tokens[ComboNodeType::MoveSpeedY]);
		//nextWeakAttack(空なら空vector)
		if (!tokens[ComboNodeType::NextLightAttack].empty())
		{
			std::istringstream weakSS(tokens[ComboNodeType::NextLightAttack]);
			std::string idx;
			while (std::getline(weakSS, idx, ';'))
			{
				node.nextWeakAttack.push_back(std::stoi(idx));
			}
		}

		//nextHeavyAttack(空なら空vector)
		if (!tokens[ComboNodeType::NextHeavyAttack].empty())
		{
			std::istringstream heavySS(tokens[ComboNodeType::NextHeavyAttack]);
			std::string idx;
			while (std::getline(heavySS, idx, ';'))
			{
				node.nextHeavyAttack.push_back(std::stoi(idx));
			}
		}
		node.knockBackXZ = std::stof(tokens[ComboNodeType::knockBackXZ]);
		node.knockBackY = std::stof(tokens[ComboNodeType::knockBackY]);
		node.isKirimomi = tokens[ComboNodeType::IsKirimomi] == "1" ? true : false;//CSVの値が1ならtrue、0ならfalse
		node.seFrameRate = std::stof(tokens[ComboNodeType::SeFrameRate]);
		node.seName = tokens[ComboNodeType::SeName];
		m_comboChain.push_back(node);
	}

}

void Player::UpdateAngle()
{
	//float targetAngle = 0.0f;//目標の角度
	//if (m_targetVec.Magnitude() > 0.0f)//最初の入力されないとき以外、ここを通り、モデルの向きを変える
	//{
	//	//プレイヤーの移動方向にモデルの方向を近づける
	//	targetAngle = atan2f(m_targetVec.x, m_targetVec.z);//移動ベクトルのx成分とz成分から、プレイヤーの向きたい方向の角度を求める
	//	// Y軸回転行列を作成する//この工程は毎フレーム、原点からプレイヤーの位置に移動してから、回転する行列を作成している
	//	//180度ずれてたので、回転角度を180度ずらす
	//	/* m_rotAngle = targetAngle - DX_PI_F;*/

	//	 //角度の差分を計算//回転角度を-90から90にするため(最短経路を選択)
	//	float difference = targetAngle - m_rotAngleY - DX_PI_F;
	//	while (difference > DX_PI_F) difference -= 2.0f * DX_PI_F;
	//	while (difference < -DX_PI_F) difference += 2.0f * DX_PI_F;
	//	//targetAngle + DX_PI_F
	//	m_rotAngleY += difference * 0.1f;//回転角度を少しずつ目標の角度に近づける//ほぼlerp
	//}	
	//	Matrix4x4 rotY = Matrix4x4::MakeRotationY(m_rotAngleY);
	//	MATRIX transmat = MGetTranslate(m_pos.ToDxLibVector());
	//	Matrix4x4 trans = Matrix4x4::FromDxLibMatrix(transmat);

	//	Matrix4x4 mtx = trans * rotY;
	//	MV1SetMatrix(m_modelHandle, Matrix4x4::ToDxLibMatrix(mtx));
}

bool Player::IsAvoidable() const
{
	if (m_avoidInfo.avoidCoolTimeCount > 0.0f)
	{
		return false;//クールタイム中は回避できない
	}

	return true;
}

void Player::WingUpdate()
{
	//モデルフレームのローカルワールド行列を取得
	MATRIX mat = MV1GetFrameLocalWorldMatrix(m_modelHandle, kPlayerNeckBoneIndex);//モデルフレームのローカルワールド行列を取得

	////武器の位置を取得
	//Vector3 weaponPos = MV1GetFramePosition(m_ownerHandle, slotIndex);

	//MATRIX transmat = MGetTranslate(weaponPos.ToDxLibVector());
	//90度回転させる
	MATRIX rotmat = MGetRotY(DX_PI_F / 2.0f);//回転行列を作成する//90度回転させる
	MATRIX rotXmat = MGetRotX(DX_PI_F / 2.0f);//回転行列を作成する//90度回転させる
	MATRIX rotZmat = MGetRotZ(DX_PI_F / 2.0f);//回転行列を作成する//90度回転させる

	rotmat = MMult(rotXmat, rotmat);//回転行列を掛ける//90度回転させる
	rotmat = MMult(rotZmat, rotmat);//回転行列を掛ける//90度回転させる
	mat = MMult(rotmat, mat);//回転行列を掛ける//90度回転させる

	MATRIX scale = MGetScale(VGet(0.35f, 0.35f, 0.35f));//スケーリング行列を作成する//モデルの大きさを半分にする

	mat = MMult(scale, mat);//スケーリング行列を掛ける//モデルの大きさを半分にする

	//オフセット
	MATRIX transmat = MGetTranslate(VGet(0.0f, -10.0f, -40.0f)); // -Z が後ろ
	//これを先に掛けることで、ローカル空間でのオフセットを適用する
	mat = MMult(transmat, mat);

	//モデルにマトリクスをセット
	MV1SetMatrix(m_wingModelHandle, mat);
}
void Player::ApplyPos()
{
	//モデルの座標を更新する
	CharacterBase::ApplyPos();
	
	WingUpdate();
	m_weapon->Update();//武器の更新
}
