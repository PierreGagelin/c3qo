#ifndef C3QO_MANAGER_HPP
#define C3QO_MANAGER_HPP

namespace manager_bk
{

bool conf_parse(const char *filename);
size_t conf_get(char *buf, size_t len);

void block_clean();

} // END namespace manager_bk

#endif // C3QO_MANAGER_HPP