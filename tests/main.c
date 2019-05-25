#include "pcunit_unittests.h"

int main(void)
{
    const PCU_SuiteMethod suites[] = {
            TaskTest_suite,
    };

    PCU_enable_color();
    PCU_set_putchar(putchar);
    PCU_set_getchar(getchar);
    PCU_set_verbose(1);
    return PCU_run(suites, sizeof suites / sizeof *suites);
}

