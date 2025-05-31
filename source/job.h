#pragma once

struct job
{
  transaction trx;
  int64_t id;
  std::string name;
  unsigned int duration;
};
