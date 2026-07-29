DO $$
DECLARE
    v_session_key text;
BEGIN
    v_session_key := user_api.log_in('Administrator', 'P2ssw0rd');

    PERFORM api.add_or_update_file(v_session_key, '$root$/Schemas', 'Actuators', 'Update: Adding file $root$/Schemas/Actuators', '', '{}');
    PERFORM api.set_file_attributes(v_session_key, '$root$/Schemas/Actuators', 1);
END;
$$;