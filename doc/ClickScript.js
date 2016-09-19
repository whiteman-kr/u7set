
// Set Property value
// item.setPropertyValue(propertyName, newValue)
//
 
// Get PropertyValue
// item.propertyValue(propertyName)
// returns property value for the item

// view.SetSchema(SchemaID)
// Sets new schema for view


// Some examples of sripts for schemas
//
	
	
// Switch schema
//
function(item, view) 
{
	// Item is VFrame30::SchemaItem class
	// View is MonitorSchemaView (deribed from VFrame30::SchemaView)
	//
	view.setSchema("AL02");
}

	
	
// Set Item's properties ClickScript
//
function(item, view) 
{
	// Item is VFrame30::SchemaItem class
	// View is MonitorSchemaView (deribed from VFrame30::SchemaView)
	item.setPropertyValue("Text", "Hi there!!!");
	item.setPropertyValue("Text", item.propertyValue("Text") + "!");
	
}