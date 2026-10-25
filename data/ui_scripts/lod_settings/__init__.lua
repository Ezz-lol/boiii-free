BoiiiLodSettings = {
  {
    dvar = "r_lodBiasRigid",
    name = "Object Detail Distance",
    description = "How far away objects keep their full detail. Higher reduces pop-in on maps like Origins, but costs performance on low-end GPUs.",
    options = {
      { option = "Default", value = 0 },
      { option = "Far", value = -250 },
      { option = "Very Far", value = -500 },
      { option = "Maximum", value = -1000 },
    },
  },
  {
    dvar = "r_modelLodBias",
    name = "Model Detail Distance",
    description = "How far away models keep their full detail. Higher reduces pop-in, but costs performance on low-end GPUs.",
    options = {
      { option = "Default", value = 1 },
      { option = "High", value = 2 },
      { option = "Very High", value = 5 },
      { option = "Maximum", value = 10 },
    },
  },
}

function BoiiiLodSettings.CreateDvarSettings(controller, prefix)
  local items = {}
  for _, setting in ipairs(BoiiiLodSettings) do
    local current = Engine.DvarInt(nil, setting.dvar)
    local options = {}
    local found = false
    for _, option in ipairs(setting.options) do
      found = found or option.value == current
      table.insert(options, { option = option.option, value = option.value, default = option.value == current })
    end
    if not found then
      table.insert(options, { option = current .. " (Custom)", value = current, default = true })
    end
    table.insert(
      items,
      CoD.OptionsUtility.CreateDvarSettings(
        controller,
        setting.name,
        setting.description,
        prefix .. setting.dvar,
        setting.dvar,
        options,
        nil,
        function(_, optionModel, _, dvarName)
          UpdateInfoModels(optionModel)
          if Engine.DvarInt(nil, dvarName) ~= optionModel.value then
            Engine.SetDvar(dvarName, optionModel.value)
          end
        end
      )
    )
  end
  return items
end
