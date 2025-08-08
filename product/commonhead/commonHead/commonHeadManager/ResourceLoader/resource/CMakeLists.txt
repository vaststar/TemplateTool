include(TemplateTools)

set(RESOURCE_IDE_FOLDER commonhead/codegen)
generate_from_template(
    TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/templates/ResourceString.h.jinja
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ResourceString.json
    OUTPUT_FILE ${CMAKE_BINARY_DIR}/ResourceString.h
    IDE_FOLDER ${RESOURCE_IDE_FOLDER}
)

add_custom_target(GenerateResourceHeader ALL DEPENDS ${CMAKE_BINARY_DIR}/ResourceString.h)
set_target_properties(GenerateResourceHeader PROPERTIES FOLDER ${RESOURCE_IDE_FOLDER})
