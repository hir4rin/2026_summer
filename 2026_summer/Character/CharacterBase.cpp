#include "CharacterBase.h"
#include <assert.h>

CharacterBase::CharacterBase()
{
}

CharacterBase::~CharacterBase()
{
}


const std::string& CharacterBase::GetAnimName(const std::string& key) const
{
	auto it = m_animNames.find(key);
	assert(it != m_animNames.end() && "指定されたキーが見つかりませんでした");
	return it->second;

}