SELECT * FROM api.set_project_property('$(SessionKey)', 'Run Simulator Tests on Build', 'true');
SELECT * FROM api.set_project_property('$(SessionKey)', 'Simulator Tests Timeout', '-1');