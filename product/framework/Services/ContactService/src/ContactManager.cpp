#include "ContactManager.h"

#include <ucf/CoreFramework/ICoreFramework.h>

#include "ContactServiceLogger.h"
#include "ContactModel.h"
#include "Adapters/ContactAdapter.h"
namespace ucf::service{

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ContactManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
ContactManager::ContactManager(ucf::framework::ICoreFrameworkWPtr coreFramework)
    : mCoreFrameworkWPtr(coreFramework)
    , mContactModelPtr(std::make_unique<ContactModel>(coreFramework))
    , mContactAdapter(std::make_unique<ucf::adapter::ContactAdapter>(coreFramework))
{
    initializeTestData();
}

ContactManager::~ContactManager()
{

}

std::vector<model::IPersonContactPtr> ContactManager::getPersonContactList() const
{
    mContactAdapter->fetchContactInfo("", [](const ucf::service::model::Contact& contact){
        SERVICE_LOG_DEBUG("test fetch contactInfo");
    });
    
    return mContactModelPtr->getPersonContacts();
}

model::IPersonContactPtr ContactManager::getPersonContact(const std::string& contactId) const
{
    return mContactModelPtr->getPersonContact(contactId);
}

model::IGroupContactPtr ContactManager::getGroupContact(const std::string& contactId) const
{
    return mContactModelPtr->getGroupContact(contactId);
}

std::vector<model::IGroupContactPtr> ContactManager::getGroupContactList() const
{
    return mContactModelPtr->getGroupContacts();
}

std::vector<model::IContactRelationPtr> ContactManager::getContactRelations() const
{
    return mContactModelPtr->getContactRelations();
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
////////////////////Start ContactManager Logic//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void ContactManager::initializeTestData()
{
    // ==============================================
    // 1. 创建组织结构（GroupContact）
    // ==============================================
    
    // 根节点 - 公司
    auto company = std::make_shared<model::GroupContact>("company_001");
    company->setGroupName("科技创新有限公司");
    mContactModelPtr->addGroupContact(company);
    
    // 一级部门
    auto techDept = std::make_shared<model::GroupContact>("dept_tech");
    techDept->setGroupName("技术部");
    mContactModelPtr->addGroupContact(techDept);
    
    auto salesDept = std::make_shared<model::GroupContact>("dept_sales");
    salesDept->setGroupName("销售部");
    mContactModelPtr->addGroupContact(salesDept);
    
    auto hrDept = std::make_shared<model::GroupContact>("dept_hr");
    hrDept->setGroupName("人事部");
    mContactModelPtr->addGroupContact(hrDept);
    
    auto financeDept = std::make_shared<model::GroupContact>("dept_finance");
    financeDept->setGroupName("财务部");
    mContactModelPtr->addGroupContact(financeDept);
    
    // 技术部下的二级部门
    auto devTeam = std::make_shared<model::GroupContact>("team_dev");
    devTeam->setGroupName("开发组");
    mContactModelPtr->addGroupContact(devTeam);
    
    auto testTeam = std::make_shared<model::GroupContact>("team_test");
    testTeam->setGroupName("测试组");
    mContactModelPtr->addGroupContact(testTeam);
    
    auto uiTeam = std::make_shared<model::GroupContact>("team_ui");
    uiTeam->setGroupName("UI设计组");
    mContactModelPtr->addGroupContact(uiTeam);
    
    // 开发组下的小组
    auto frontendTeam = std::make_shared<model::GroupContact>("team_frontend");
    frontendTeam->setGroupName("前端开发小组");
    mContactModelPtr->addGroupContact(frontendTeam);
    
    auto backendTeam = std::make_shared<model::GroupContact>("team_backend");
    backendTeam->setGroupName("后端开发小组");
    mContactModelPtr->addGroupContact(backendTeam);
    
    // ==============================================
    // 2. 创建员工（PersonContact）
    // ==============================================
    
    // 高管层
    auto ceo = std::make_shared<model::PersonContact>("person_ceo");
    ceo->setPersonName("张总");
    mContactModelPtr->addPersonContact(ceo);
    
    // 部门负责人
    auto techManager = std::make_shared<model::PersonContact>("person_tech_mgr");
    techManager->setPersonName("李技术");
    mContactModelPtr->addPersonContact(techManager);
    
    auto salesManager = std::make_shared<model::PersonContact>("person_sales_mgr");
    salesManager->setPersonName("王销售");
    mContactModelPtr->addPersonContact(salesManager);
    
    auto hrManager = std::make_shared<model::PersonContact>("person_hr_mgr");
    hrManager->setPersonName("陈人事");
    mContactModelPtr->addPersonContact(hrManager);
    
    auto financeManager = std::make_shared<model::PersonContact>("person_finance_mgr");
    financeManager->setPersonName("赵财务");
    mContactModelPtr->addPersonContact(financeManager);
    
    // 组长
    auto devLead = std::make_shared<model::PersonContact>("person_dev_lead");
    devLead->setPersonName("赵开发");
    mContactModelPtr->addPersonContact(devLead);
    
    auto testLead = std::make_shared<model::PersonContact>("person_test_lead");
    testLead->setPersonName("孙测试");
    mContactModelPtr->addPersonContact(testLead);
    
    auto uiLead = std::make_shared<model::PersonContact>("person_ui_lead");
    uiLead->setPersonName("周设计");
    mContactModelPtr->addPersonContact(uiLead);
    
    // 小组负责人
    auto frontendLead = std::make_shared<model::PersonContact>("person_frontend_lead");
    frontendLead->setPersonName("吴前端");
    mContactModelPtr->addPersonContact(frontendLead);
    
    auto backendLead = std::make_shared<model::PersonContact>("person_backend_lead");
    backendLead->setPersonName("郑后端");
    mContactModelPtr->addPersonContact(backendLead);
    
    // 前端开发人员
    auto frontendDev1 = std::make_shared<model::PersonContact>("person_frontend_001");
    frontendDev1->setPersonName("钱小前");
    mContactModelPtr->addPersonContact(frontendDev1);
    
    auto frontendDev2 = std::make_shared<model::PersonContact>("person_frontend_002");
    frontendDev2->setPersonName("刘小端");
    mContactModelPtr->addPersonContact(frontendDev2);
    
    auto frontendDev3 = std::make_shared<model::PersonContact>("person_frontend_003");
    frontendDev3->setPersonName("陈小前");
    mContactModelPtr->addPersonContact(frontendDev3);
    
    // 后端开发人员
    auto backendDev1 = std::make_shared<model::PersonContact>("person_backend_001");
    backendDev1->setPersonName("何小后");
    mContactModelPtr->addPersonContact(backendDev1);
    
    auto backendDev2 = std::make_shared<model::PersonContact>("person_backend_002");
    backendDev2->setPersonName("许小端");
    mContactModelPtr->addPersonContact(backendDev2);
    
    auto backendDev3 = std::make_shared<model::PersonContact>("person_backend_003");
    backendDev3->setPersonName("韩小后");
    mContactModelPtr->addPersonContact(backendDev3);
    
    // 测试人员
    auto tester1 = std::make_shared<model::PersonContact>("person_test_001");
    tester1->setPersonName("朱小测");
    mContactModelPtr->addPersonContact(tester1);
    
    auto tester2 = std::make_shared<model::PersonContact>("person_test_002");
    tester2->setPersonName("徐小试");
    mContactModelPtr->addPersonContact(tester2);
    
    auto tester3 = std::make_shared<model::PersonContact>("person_test_003");
    tester3->setPersonName("马小检");
    mContactModelPtr->addPersonContact(tester3);
    
    // UI设计人员
    auto designer1 = std::make_shared<model::PersonContact>("person_ui_001");
    designer1->setPersonName("韩小美");
    mContactModelPtr->addPersonContact(designer1);
    
    auto designer2 = std::make_shared<model::PersonContact>("person_ui_002");
    designer2->setPersonName("冯小艺");
    mContactModelPtr->addPersonContact(designer2);
    
    // 销售人员
    auto salesperson1 = std::make_shared<model::PersonContact>("person_sales_001");
    salesperson1->setPersonName("杨小销");
    mContactModelPtr->addPersonContact(salesperson1);
    
    auto salesperson2 = std::make_shared<model::PersonContact>("person_sales_002");
    salesperson2->setPersonName("田小售");
    mContactModelPtr->addPersonContact(salesperson2);
    
    // 人事专员
    auto hrSpecialist = std::make_shared<model::PersonContact>("person_hr_001");
    hrSpecialist->setPersonName("范小人");
    mContactModelPtr->addPersonContact(hrSpecialist);
    
    // 财务专员
    auto financeSpecialist = std::make_shared<model::PersonContact>("person_finance_001");
    financeSpecialist->setPersonName("邓小财");
    mContactModelPtr->addPersonContact(financeSpecialist);
    
    // ==============================================
    // 3. 建立组织关系（只使用 Department 类型）
    // ==============================================
    
    // 公司 -> 一级部门
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("dept_tech", "company_001"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("dept_sales", "company_001"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("dept_hr", "company_001"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("dept_finance", "company_001"));
    
    // 技术部 -> 二级部门
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("team_dev", "dept_tech"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("team_test", "dept_tech"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("team_ui", "dept_tech"));
    
    // 开发组 -> 开发小组
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("team_frontend", "team_dev"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("team_backend", "team_dev"));
    
    // ==============================================
    // 4. 建立人员归属关系（人员属于部门/组织）
    // ==============================================
    
    // CEO 属于公司
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_ceo", "company_001"));
    
    // 部门经理属于各自部门
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_tech_mgr", "dept_tech"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_sales_mgr", "dept_sales"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_hr_mgr", "dept_hr"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_finance_mgr", "dept_finance"));
    
    // 组长属于各自组织
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_dev_lead", "team_dev"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_test_lead", "team_test"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_ui_lead", "team_ui"));
    
    // 小组长属于各自小组
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_frontend_lead", "team_frontend"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_backend_lead", "team_backend"));
    
    // 前端开发人员属于前端小组
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_frontend_001", "team_frontend"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_frontend_002", "team_frontend"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_frontend_003", "team_frontend"));
    
    // 后端开发人员属于后端小组
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_backend_001", "team_backend"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_backend_002", "team_backend"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_backend_003", "team_backend"));
    
    // 测试人员属于测试组
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_test_001", "team_test"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_test_002", "team_test"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_test_003", "team_test"));
    
    // UI设计人员属于UI设计组
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_ui_001", "team_ui"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_ui_002", "team_ui"));
    
    // 销售人员属于销售部
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_sales_001", "dept_sales"));
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_sales_002", "dept_sales"));
    
    // 人事专员属于人事部
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_hr_001", "dept_hr"));
    
    // 财务专员属于财务部
    mContactModelPtr->addContactRelation(std::make_shared<model::ContactRelation>("person_finance_001", "dept_finance"));
    
    SERVICE_LOG_INFO("Test organization structure created successfully!");
    printOrganizationStructure();
}

void ContactManager::printOrganizationStructure() const
{
    SERVICE_LOG_INFO("=== Organization Structure ===");
    
    // 打印所有人员
    auto persons = mContactModelPtr->getPersonContacts();
    SERVICE_LOG_INFO("Persons (" << persons.size() << "):");
    for (const auto& person : persons) {
        SERVICE_LOG_INFO("  " << person->getPersonName() << " (" << person->getContactId() << ")");
    }
    
    // 打印所有组织
    auto groups = mContactModelPtr->getGroupContacts();
    SERVICE_LOG_INFO("Groups (" << groups.size() << "):");
    for (const auto& group : groups) {
        SERVICE_LOG_INFO("  " << group->getGroupName() << " (" << group->getContactId() << ")");
    }
    
    // 打印所有关系（现在都是 Department 类型）
    auto relations = mContactModelPtr->getContactRelations();
    SERVICE_LOG_INFO("Relations (" << relations.size() << "):");
    for (const auto& relation : relations) {
        SERVICE_LOG_INFO("  " << relation->getChildId() << " belongs to " << relation->getParentId());
    }
}
}