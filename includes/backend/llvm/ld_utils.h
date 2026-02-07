/*
 * ld_utils: contains functions for the ld.lld linker
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LD_UTILS
#define LD_UTILS

/*
 * @brief: helper function to link the generated output object file to an
 * executable binary.
 *
 * @param output_file: name to be given to the output executable binary.
 * @param obj_file: name of the generated output object file.
 */
void ld_link(const char *output_file, const char *obj_file);

#ifdef __cplusplus
}
#endif

#endif // !LD_UTILS
