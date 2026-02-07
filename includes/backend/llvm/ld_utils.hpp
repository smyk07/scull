/*
 * ld_utils: contains functions for the ld.lld linker
 */

#ifndef LD_UTILS
#define LD_UTILS

#include <vector>

/*
 * @brief: helper function to link the generated output object file to an
 * executable binary.
 *
 * @param output_file: name to be given to the output executable binary.
 * @param obj_files: vector of object files to be linked.
 */
void ld_link(const char *output_file,
             const std::vector<const char *> &obj_files);

#endif // !LD_UTILS
