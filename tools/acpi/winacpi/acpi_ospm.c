#include "winacpi.h"

#define ACPI_MAX_TABLES		128
static struct acpi_table_desc acpi_gbl_initial_tables[ACPI_MAX_TABLES];
static struct acpi_table_desc *acpi_gbl_initial_tables_ids[ACPI_MAX_TABLES];

void acpi_ospm_init(void)
{
	int i;

	acpi_initialize_subsystem();
	for (i = 0; i < ACPI_MAX_TABLES; i++)
		acpi_gbl_initial_tables_ids[i] = &acpi_gbl_initial_tables[i];
	acpi_initialize_namespace();
	acpi_initialize_tables(acpi_gbl_initial_tables_ids, ACPI_MAX_TABLES, false);
	acpi_reallocate_root_table();
	acpi_load_tables();
#if 0
	acpi_enable_subsystem(~ACPI_NO_ACPI_ENABLE);
	acpi_os_install_handlers();
	acpi_enable_subsystem(ACPI_NO_ACPI_ENABLE);
	acpi_initialize_objects(ACPI_FULL_INITIALIZATION);
#endif
}

void acpi_ospm_exit(void)
{
	acpi_finalize_tables();
}
