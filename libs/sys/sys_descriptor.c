#include "sys_descriptor.h"
#include "sys_compiler.h"

#include "sys_log.h"
#define TAG "INFO"

const __attribute__((section(".sys_descriptor"))) __attribute((weak))
sys_descriptor_t sys_descriptor = {
                                    .magic_word = APP_DESCRIPTOR_MAGIC_WORD,
                                   .project_name = PROJECT_NAME,
                                   .version = VESRION_STRING,
                                   .build_date = COMPILE_DATE,
                                   .build_describe = GIT_COMMIT_INFO,
                                   .moduleId = PROJECT_MODULE_ID,
                                   .hwVersionMajor = PROJECT_HW_V_MAJOR,
                                   .hwVersionMinor = PROJECT_HW_V_MINOR,
                                   .hwVersionPatch = PROJECT_HW_V_PATCH,
                                //    .fwVersionMajor = GIT_VERSION_MAJOR,
                                //    .fwVersionMinor = GIT_VERSION_MINOR,
                                //    .fwVersionPatch = GIT_VERSION_PATCH,

                                   /* crc32 is overwritten by the build process */
                                   .crc32 = 0xFFFFFFFF};

#ifndef __CDT_PARSER__
_Static_assert(sizeof(COMPILE_DATE) <= sizeof(sys_descriptor.build_date),
               "APP_BUILD_DATE is longer than version field in structure");
_Static_assert(sizeof(GIT_COMMIT_INFO) <= sizeof(sys_descriptor.build_describe),
               "APP_BUILD_DESCRIBE is longer than version field in structure");
#endif

const sys_descriptor_t* sys_descriptor_get() {
    return &sys_descriptor;
}

void decriptor_log(const sys_descriptor_t* app_descr) {
    log_i(TAG, "project: %s", app_descr->project_name);
    log_i(TAG, "moduleId: %d", app_descr->moduleId);
    log_i(TAG, "fwVersion: %d.%d.%d", app_descr->fwVersionMajor, app_descr->fwVersionMinor, app_descr->fwVersionPatch);
    log_i(TAG, "hwVersion: %d.%d.%d", app_descr->hwVersionMajor, app_descr->hwVersionMinor, app_descr->hwVersionPatch);
    log_i(TAG, "Git info: %s", app_descr->version);
    log_i(TAG, "Build: %08lX, %s", __builtin_bswap32(app_descr->crc32), app_descr->build_date);

    // log_i(TAG, "Build describe: %s", app_descr->build_describe);
}
