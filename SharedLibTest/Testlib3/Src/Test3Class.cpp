#include "Testlib3/Test3Class.h"

namespace Testlib3
{

Test3Class::Test3Class()
    : m_entityId(0)
    , m_entityName("DefaultTest3Entity")
    , m_info("A test entity from Testlib3")
    , m_count(0)
    , m_factor(1.0)
    , m_active(true)
    , m_connectionState(0)  // Disconnected
    , m_priority(1)           // Normal
{
}

}
