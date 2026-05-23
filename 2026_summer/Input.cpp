#include "Input.h"
#include "Dxlib.h"

Input::Input() 
{
   
}
void Input::Init()
{
    //初期化
    m_inputData = {}, m_lastInputData = {}, m_inputTable = {};

    //イベント名を添え字にして、右辺値に実際の入力種別と入力コードの配列をおく
    m_inputTable["A"] = { { PeripheralType::keyboard,KEY_INPUT_Z },
                          { PeripheralType::pad1,PAD_INPUT_1 } };//PADのAボタン

    m_inputTable["B"] = { { PeripheralType::keyboard,KEY_INPUT_RETURN },
                          { PeripheralType::pad1,PAD_INPUT_2 } };//PADのBボタン

    m_inputTable["X"] = { { PeripheralType::keyboard,KEY_INPUT_X },
                          { PeripheralType::pad1,PAD_INPUT_3 } };//PADのXボタン

    m_inputTable["Y"] = { { PeripheralType::keyboard,KEY_INPUT_C },
                          { PeripheralType::pad1,PAD_INPUT_4 } };//PADのYボタン


    m_inputTable["="] = { { PeripheralType::keyboard,KEY_INPUT_V },
                          { PeripheralType::pad1,PAD_INPUT_7 } };//PADのSボタン

    m_inputTable["Start"] = { { PeripheralType::keyboard,KEY_INPUT_P },
                          { PeripheralType::pad1,PAD_INPUT_8 } };//SELECTキー

    m_inputTable["Up"] = { { PeripheralType::keyboard,KEY_INPUT_UP },
                          { PeripheralType::pad1,PAD_INPUT_UP} };
    m_inputTable["Down"] = { { PeripheralType::keyboard,KEY_INPUT_DOWN },
                            { PeripheralType::pad1,PAD_INPUT_DOWN} };
    m_inputTable["Left"] = { { PeripheralType::keyboard,KEY_INPUT_LEFT },
                            { PeripheralType::pad1,PAD_INPUT_LEFT} };
    m_inputTable["Right"] = { { PeripheralType::keyboard,KEY_INPUT_RIGHT },
                             { PeripheralType::pad1,PAD_INPUT_RIGHT} };

    // あらかじめ枠を開けておく
    //ここで枠を開けておかないと、チェックの際にAt関数でクラッシュする可能性がある
    for (const auto& inputInfo : m_inputTable) {
        m_inputData[inputInfo.first] = false;
        m_lastInputData[inputInfo.first] = false;
    }
}

bool Input::IsLeftStickInput()
{
	//十字キーのいずれかが押されているか
    if(IsPressed("Up") || IsPressed("Down") || IsPressed("Left") || IsPressed("Right"))
    {
        return true;
	}

    return false;
}

void Input::Update()
{
    // まず現在の入力情報を取得
    char keyState[256];
    GetHitKeyStateAll(keyState);//生のキーボード情報//この関数が入力を全部とってくる(keyStateに入れてる)
    int padState = GetJoypadInputState(DX_INPUT_PAD1);//生のPAD1情報
    m_lastInputData = m_inputData;//直前のフレームを更新(前のフレーム情報をコピー)

    // すべての入力イベントをチェックします
    //ここでInputData_が更新される
    //inputTable_を回して各イベントをチェックする
    for (const auto&inputInfo : m_inputTable)
    {
        auto& input=m_inputData[inputInfo.first];//inputInfo.firstには"ok"等が入っている
        //inputを書き換えると、inputData_のそのイベントが押されているかどうかを
        //書き換えることになる
        //InputStateのベクタを回す
        for (const auto& state : inputInfo.second){
            //このループはInputState配列のループなので
            //まず、入力種別をチェックします
            switch (state.type)
            {
            case PeripheralType::keyboard://キーボードだったら
                //GethitKeyStateAllでとってきた配列の中身を見て
                //入力されているかどうかをチェックします
                input = keyState[state.id];
                break;
            case PeripheralType::pad1:
                //GethitKeyStateAllでとってきたビット情報を見て
                //登録されているビット情報と&をとり、そのビットが立っているかどうかをチェック
                input = (padState & state.id);
                break;
            }
            if (input) {//必須!
                break;//ここでbreakしないと、最後のチェックで押されていないとfalseになる
            }
        }
    }

}

bool Input::IsPressed(const char* name) const
{
    //もし「ない」イベントを送られるとクラッシュします
    //クラッシュがいやだったら
    //if (inputData_.contains(name))
    //{
    //    return false;//これで回避できます
    //}
    return m_inputData.at(name);//const関数内部なので[]ではなくatを使用している
}

bool Input::IsTriggered(const char* name) const
{
    return m_inputData.at(name) && !m_lastInputData.at(name);
}
