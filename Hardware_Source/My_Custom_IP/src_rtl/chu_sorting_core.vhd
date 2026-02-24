----------------------------------------------------------------------------------
-- Company: George Mason
-- Engineer: Kainoa Asse
-- Create Date: 04/26/2024 01:10:18 PM
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: chu_sorting_core - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------
--Top-Level Wrapper matching the interface of an MMIO core

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity chu_sorting_core is
    Port (clk     : in  std_logic; 
          reset   : in  std_logic; 
          -- io bridge interface
          cs      : in  std_logic; 
          write   : in  std_logic;
          read    : in  std_logic;
          addr    : in  std_logic_vector(4 downto 0);
          rd_data : out std_logic_vector(31 downto 0);
          wr_data : in  std_logic_vector(31 downto 0));

end chu_sorting_core;

architecture Behavioral of chu_sorting_core is
    signal s, MigtMj, zi, zj, Wr, Li, Ei, Lj, Ej : std_logic;
    signal Wrs, WrN, Wrl, WrInit, WrMem, Rd : std_logic;
    signal Eri, Lri : std_logic;
    signal Done : std_logic;
    signal temp : std_logic_vector(2 downto 0);
    signal DataOut : std_logic_vector(15 downto 0);
    signal s_reg : std_logic;
    signal write_reg : std_logic_vector(31 downto 0);
    signal n_reg : std_logic_vector(15 downto 0);
    signal WrInit_r, Rd_r, RdMem, RdStatus : std_logic;
    signal ri : std_logic_vector(12 downto 0);
begin

    --instantiation of sorting datapath
    sort_datapath_unit : entity work.Sorting_datapath
        Port Map(clk => clk,
                 DataIn => write_reg(15 downto 0),
                 RAdd => ri,
                 N_in => n_reg,
                 WrInit => WrInit,
                 Rd => Rd,
                 Wr => Wr,
                 Li => Li,
                 Ei => Ei,
                 Lj => Lj,
                 Ej => Ej,
                 MigtMj => MigtMj,
                 zi => zi,
                 zj => zj,
                 s => s,
                 Done => Done,
                 addr_ctrl => addr(2),              
                 DataOut => DataOut);
    --slot interface 
    rd_data(15 downto 0) <= DataOut;    
    rd_data(31 downto 16) <= (others => '0'); 
    
    -- input data register process
    process(clk, reset)
    begin
        if (reset = '1') then
            write_reg <= (others => '0');
        elsif rising_edge(clk) then
            if (WrInit = '1') then
                write_reg <= wr_data;
            end if;
        end if;
    end process;
    
    -- ri counter
   ri_counter : entity work.addr_counter
        Port Map (clk => clk,
                  en => Eri,
                  ld => Lri,
                  Q => ri);
   Eri <= WrMem or RdMem;
   Lri <= Wrl;
                 
   -- signal s register
    process(clk, reset)
    begin
        if (reset = '1') then
            s <= '0';
        elsif rising_edge(clk) then
            if (Wrs = '1') then
                s <= wr_data(0);
            end if;
        end if;
    end process;
    
   -- input N address register
    process(clk, reset)
    begin
        if (reset = '1') then
            n_reg <= (others => '0');
        elsif rising_edge(clk) then 
            if (WrN = '1') then 
                n_reg <= wr_data(15 downto 0);
            end if;
        end if;
    end process;
   
   -- WrInit register and logic
   process(clk, reset)
   begin
        if (reset = '1') then
            WrInit_r <= '0';
        elsif rising_edge(clk) then
            if (Wrl = '1') then
                WrInit_r <= wr_data(2);
            end if;
        end if;
   end process;  
   WrInit <= WrInit_r and WrMem;
   
   -- Rd register and logic
   process(clk, reset)
   begin
        if (reset = '1') then
            Rd_r <= '0';
        elsif rising_edge(clk) then
            if (Wrl = '1') then
                Rd_r <= not wr_data(2);
            end if;
        end if;
   end process;
   Rd <= (Rd_r and RdMem) or RdStatus; 
            
   --instantiation of controller
   sort_controller_unit : entity work.controller
        Port Map(clk => clk,
                 reset => reset,
                 s => s,
                 MigtMj => MigtMj,
                 zi => zi,
                 zj => zj,
                 Wr => Wr,
                 Li => Li,
                 Ei => Ei,
                 Lj => Lj,
                 Ej => Ej,
                 Done => Done);
            
    --Combinational logic for MMIO wrapper control signals
    temp <= cs & write & read;
    WrN <= '1' when (temp = "110") and (addr(2 downto 0) = "010") else '0';
    Wrs <= '1' when (temp = "110") and (addr(2 downto 0) = "011") and (wr_data(1) = '0') else '0';
    Wrl <= '1' when (temp = "110") and (addr(2 downto 0) = "011") and (wr_data(1 downto 0) = "10") else '0';
    WrMem <= '1' when (temp = "110") and (addr(2 downto 0) = "000") else '0';
    RdMem <= '1' when (temp = "101") and (addr(2 downto 0) = "001") else '0';
    RdStatus <= '1' when (temp = "101") and (addr(2 downto 0) = "100") else '0';
    
end Behavioral;
