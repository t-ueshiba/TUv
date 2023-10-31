/*
 *  $Id$  
 */
#include "TU/v/TUv++.h"
#include <functional>
#include <algorithm>
#include <stdexcept>

namespace TU
{
namespace v
{
/************************************************************************
*  class CmdParent							*
************************************************************************/
/*
 *  public member functions.
 */
CmdVal
CmdParent::getValue(CmdId id) const
{
    const Cmd*	vcmd = findDescendant(id);
    if (vcmd == 0)
	throw std::domain_error("TU::v::CmdParent::getValue: command not found!!");
    return vcmd->getValue();
}

void
CmdParent::setValue(CmdId id, CmdVal val)
{
    Cmd*	vcmd = findDescendant(id);
    if (vcmd == 0)
	throw std::domain_error("TU::v::CmdParent::setValue: command not found!!");
    vcmd->setValue(val);
}

const char*
CmdParent::getString(CmdId id) const
{
    const Cmd*	vcmd = findDescendant(id);
    if (vcmd == 0)
	throw std::domain_error("TU::v::CmdParent::getString: command not found!!");
    return vcmd->getString();
}

void
CmdParent::setString(CmdId id, const char* str)
{
    Cmd*	vcmd = findDescendant(id);
    if (vcmd == 0)
	throw std::domain_error("TU::v::CmdParent::setString: command not found!!");
    vcmd->setString(str);
}

void
CmdParent::setProp(CmdId id, const void* prop)
{
    Cmd*	vcmd = findDescendant(id);
    if (vcmd == 0)
	throw std::domain_error("TU::v::CmdParent::setProp: command not found!!");
    vcmd->setProp(prop);
}

/*
 *  protected member functions.
 */
const Cmd*
CmdParent::findChild(CmdId id) const
{
    using namespace std;
    using namespace std::placeholders;
    
    List<Cmd>::ConstIterator where = find_if(_cmdList.begin(), _cmdList.end(),
					     [id](const auto& vcmd)
					     { return vcmd.id() == id; });
    return (where != _cmdList.end() ? where.operator ->() : 0);
}

Cmd*
CmdParent::findChild(CmdId id)
{
    using namespace std;
    
    List<Cmd>::Iterator where = find_if(_cmdList.begin(), _cmdList.end(),
					[id](const auto& vcmd)
					{ return vcmd.id() == id; });
    return (where != _cmdList.end() ? where.operator ->() : 0);
}

const Cmd*
CmdParent::findChildWithNonZeroValue() const
{
    using namespace std;
    
    List<Cmd>::ConstIterator where = find_if(_cmdList.begin(), _cmdList.end(),
					     [](const auto& vcmd)
					     { return vcmd.getValue() != 0; });
    return (where != _cmdList.end() ? where.operator ->() : 0);
}

/*
 *  private member functions.
 */
const Cmd*
CmdParent::findDescendant(CmdId id) const
{
    const Cmd*	vcmd = findChild(id);

    if (vcmd != 0)
	return vcmd;

    for (List<Cmd>::ConstIterator iter = _cmdList.begin();
	 iter != _cmdList.end(); ++iter)
	if ((vcmd = iter->findDescendant(id)) != 0)
	    return vcmd;

    return 0;
}

Cmd*
CmdParent::findDescendant(CmdId id)
{
    Cmd*	vcmd = findChild(id);

    if (vcmd != 0)
	return vcmd;

    for (List<Cmd>::Iterator iter = _cmdList.begin();
	 iter != _cmdList.end(); ++iter)
	if ((vcmd = iter->findDescendant(id)) != 0)
	    return vcmd;

    return 0;
}

}
}
